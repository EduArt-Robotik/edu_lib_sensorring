#include "SocketCANFD.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <fcntl.h>
#include <net/if.h>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>
#include <stdexcept>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace eduart::transport::protocol;

namespace eduart {

namespace sensorring {

namespace com {

SocketCANFD::SocketCANFD(std::string interface_name)
    : ComInterface({ InterfaceType::SocketCan, interface_name })
    , _soc(0)
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
    if (!send(static_cast<canid_t>(canFrame.id), canFrame.data)) {
      return false;
    }
  }
  return true;
}

bool SocketCANFD::send(canid_t canid, const std::vector<uint8_t>& tx_buf) {

  if (tx_buf.size() <= CANFD_MAX_DLEN) {
    auto frame    = std::make_unique<canfd_frame>();
    frame->can_id = canid;
    frame->len    = tx_buf.size();

    std::copy_n(tx_buf.begin(), tx_buf.size(), frame->data);
    return send(frame.get());
  }

  return false;
}

bool SocketCANFD::send(const canfd_frame* frame) {
  if (frame) {
    static double _timeCom = 0.0;
    timeval clock;
    double now = 0.0;
    do {
      ::gettimeofday(&clock, 0);
      now = static_cast<double>(clock.tv_sec) + static_cast<double>(clock.tv_usec) * 1.0e-6;
    } while ((now - _timeCom) < 0.002);
    _timeCom = now;

    int retval;

    {
      LockGuard guard(_mutex);
      retval = write(_soc, frame, sizeof(canfd_frame));
    }

    if (retval != sizeof(canfd_frame)) {
      _communication_error = true;
      throw std::runtime_error("CAN transmission error for command " + std::to_string((int)(frame->data[0])) + ", returned " + std::to_string(retval) + " submitted bytes instead of " + std::to_string(sizeof(canfd_frame)));
      return false;
    }

    _communication_error = false;
    return true;
  }
  return false;
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