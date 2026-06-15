#include "SocketCANFD.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <fcntl.h>
#include <net/if.h>
#include <poll.h>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>
#include <sensorring_transport/can/CanFdDlc.hpp>
#include <stdexcept>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace com {

namespace {
// Bound how long one send operation may wait under TX queue pressure.
constexpr int TX_BACKPRESSURE_POLL_TIMEOUT_MS = 50;
constexpr int TX_BACKPRESSURE_MAX_RETRIES     = 20;
} // namespace

SocketCANFD::SocketCANFD(std::string interface_name)
    : ComInterface({ InterfaceType::SocketCan, interface_name })
    , _soc(0)
    , _enable_brs(false)
    , _assembler(sensorring::transport::can::CanCodec::MAX_PAYLOAD_PER_FRAME)
    , _reassembler([this](const sensorring::transport::TransportFrame& frame) {
      ComEndpoint ep{ static_cast<Direction>(frame.direction), frame.boardAddress, frame.deviceId };
      dispatchMessage(ep, frame.command, frame.data);
    }) {

  try {
    openInterface();
  } catch (std::runtime_error& e) {
    closeInterface();
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open interface " + _id.name + ": " + e.what());
  }

  startListener();
}

SocketCANFD::SocketCANFD(const SocketCanParams& params)
    : ComInterface({ InterfaceType::SocketCan, params.name })
    , _soc(0)
    , _enable_brs(params.enable_brs)
    , _assembler(sensorring::transport::can::CanCodec::MAX_PAYLOAD_PER_FRAME)
    , _reassembler([this](const sensorring::transport::TransportFrame& frame) {
      ComEndpoint ep{ static_cast<Direction>(frame.direction), frame.boardAddress, frame.deviceId };
      dispatchMessage(ep, frame.command, frame.data);
    }) {

  try {
    openInterface();
  } catch (std::runtime_error& e) {
    closeInterface();
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open interface " + _id.name + ": " + e.what());
  }

  startListener();
}

SocketCANFD::~SocketCANFD() {
  stopListener();
  closeInterface();
}

bool SocketCANFD::openInterface() {
  ifreq ifr;
  sockaddr_can addr;

  _soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (_soc < 0) {
    return false;
  }

  int canfd_enable = 1;
  if (setsockopt(_soc, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_enable, sizeof(canfd_enable)) < 0) {
    throw std::runtime_error("Unable to set CAN FD mode");
  }

  strcpy(ifr.ifr_name, _id.name.c_str());
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';

  // Get interface flags
  if (ioctl(_soc, SIOCGIFFLAGS, &ifr) < 0) {
    throw std::runtime_error("Unable to get interface flags. Is the hardware connected?");
  }

  if (!(ifr.ifr_flags & IFF_UP)) {
    throw std::runtime_error("CAN interface \"" + _id.name + "\" is DOWN");
  }

  if (!(ifr.ifr_flags & IFF_RUNNING)) {
    throw std::runtime_error("CAN interface \"" + _id.name + "\" is UP but not RUNNING (no carrier)");
  }

  // Get interface index
  if (ioctl(_soc, SIOCGIFINDEX, &ifr) < 0) {
    throw std::runtime_error("Unable to get interface index");
  }
  addr.can_family  = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  fcntl(_soc, F_SETFL, O_NONBLOCK);

  if (bind(_soc, (sockaddr*)&addr, sizeof(addr)) < 0) {
    throw std::runtime_error("Unable to bind socket: " + std::string(strerror(errno)) + " [" + std::to_string(errno) + "]");
  }

  _communication_error = false;
  return true;
}

bool SocketCANFD::send(ComEndpoint target, std::uint8_t command, const std::vector<uint8_t>& data) {

  auto frames = _assembler.assemble(static_cast<sensorring::transport::Direction>(target.direction), target.boardAddress, target.deviceId, command, data);

  for (const auto& frame : frames) {
    auto canFrame = sensorring::transport::can::CanCodec::encode(frame);
    if (!sendCanFrame(canFrame.id, canFrame.data, true)) {
      return false;
    }
  }
  return true;
}

bool SocketCANFD::sendCanFrame(std::uint32_t can_id, const std::vector<uint8_t>& data, bool fd) {
  // Non-blocking SocketCAN can return EAGAIN/ENOBUFS when kernel/device TX
  // queues are full. In that case, wait for POLLOUT and retry a bounded number
  // of times instead of throttling every frame with a fixed delay.
  auto writeWithBackpressure = [this](const void* frame, std::size_t frame_size) -> bool {
    for (int attempt = 0; attempt < TX_BACKPRESSURE_MAX_RETRIES; ++attempt) {
      const int retval = write(_soc, frame, frame_size);
      if (retval == static_cast<int>(frame_size)) {
        return true;
      }

      if (retval < 0 && (errno == EAGAIN || errno == ENOBUFS)) {
        pollfd pfd{};
        pfd.fd     = _soc;
        pfd.events = POLLOUT;

        // Wait until the socket becomes writable again.
        const int poll_result = poll(&pfd, 1, TX_BACKPRESSURE_POLL_TIMEOUT_MS);
        if (poll_result > 0 && (pfd.revents & POLLOUT) != 0) {
          continue;
        }
        if (poll_result == 0 || (poll_result > 0 && (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)) {
          continue;
        }
      }
      break;
    }
    return false;
  };

  {
    LockGuard guard(_mutex);
    if (fd) {
      if (data.size() > CANFD_MAX_DLEN) {
        return false;
      }
      // Round the payload length up to the next canonical CAN FD frame size
      const std::uint8_t dlc_code  = sensorring::transport::can::bytesToDlcCode(data.size());
      const std::size_t padded_len = sensorring::transport::can::dlcCodeToBytes(dlc_code);

      canfd_frame frame{};
      frame.can_id = static_cast<canid_t>(can_id);
      frame.len    = static_cast<__u8>(padded_len);
      frame.flags  = _enable_brs ? CANFD_BRS : 0;
      std::copy_n(data.begin(), data.size(), frame.data);
      // bytes data.size()..padded_len-1 stay zeroed from canfd_frame{}.
      if (!writeWithBackpressure(&frame, sizeof(canfd_frame))) {
        _communication_error = true;
        throw std::runtime_error("CAN FD transmission error: write failed after retrying backpressure conditions");
      }
    } else {
      if (data.size() > CAN_MAX_DLEN) {
        return false;
      }
      can_frame frame{};
      frame.can_id  = static_cast<canid_t>(can_id);
      frame.can_dlc = static_cast<__u8>(data.size());
      std::copy_n(data.begin(), data.size(), frame.data);
      if (!writeWithBackpressure(&frame, sizeof(can_frame))) {
        _communication_error = true;
        throw std::runtime_error("CAN transmission error: write failed after retrying backpressure conditions");
      }
    }
  }

  _communication_error = false;
  return true;
}

bool SocketCANFD::listener() {
  _shut_down_listener = false;

  canfd_frame frame_rd;
  int recvbytes = 0;

  timeval timeout = { 0, 100 };
  fd_set readSet;

  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Starting can listener on interface " + _id.name);

  _listener_is_running = true;
  while (!_shut_down_listener) {
    FD_ZERO(&readSet);

    {
      LockGuard guard(_mutex);
      FD_SET(_soc, &readSet);
      if (select((_soc + 1), &readSet, NULL, NULL, &timeout) >= 0) {
        if (FD_ISSET(_soc, &readSet)) {
          recvbytes = read(_soc, &frame_rd, sizeof(canfd_frame));
          if (recvbytes && frame_rd.len >= HEADER_SIZE) {
            std::uint8_t sysId = (frame_rd.can_id >> 8) & 0x07;
            if (sysId == sensorring::transport::can::CanCodec::SYSID_SENSOR_RING) {
              auto transportFrame = sensorring::transport::can::CanCodec::decode(frame_rd.can_id, frame_rd.data, frame_rd.len);
              _reassembler.processFrame(transportFrame);
            }
          }
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Stopping can listener on interface " + _id.name);

  _listener_is_running = false;
  return true;
}

bool SocketCANFD::closeInterface() {
  bool retval = false;
  if (_soc) {
    retval = (close(_soc) == 0);
  }
  return retval;
}

bool SocketCANFD::repairInterface() {
  stopListener();
  closeInterface();

  if (openInterface()) {
    if (startListener())
      return true;
  }
  return false;
}

} // namespace com

} // namespace sensorring

} // namespace eduart