#include "SocketCANFD.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <fcntl.h>
#include <net/if.h>
#include <stdexcept>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

#include "interface/ComEndpoints.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace com {

SocketCANFD::SocketCANFD(std::string interface_name)
    : ComInterface()
    , _soc(0) {

  try {
    openInterface(interface_name);
  } catch (std::runtime_error& e) {
    closeInterface();
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open interface " + _interface_name + ": " + e.what());
  }

  _endpoints = ComEndpoint::createStaticEndpoints();
  fillEndpointMap();
  startListener();
}

SocketCANFD::~SocketCANFD() {
  stopListener();
  closeInterface();
}

bool SocketCANFD::openInterface(std::string interface_name) {
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

  strcpy(ifr.ifr_name, interface_name.c_str());
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';

  // Get interface flags
  if (ioctl(_soc, SIOCGIFFLAGS, &ifr) < 0) {
    throw std::runtime_error("Unable to get interface flags. Is the hardware connected?");
  }

  if (!(ifr.ifr_flags & IFF_UP)) {
    throw std::runtime_error("CAN interface \"" + interface_name + "\" is DOWN");
  }

  if (!(ifr.ifr_flags & IFF_RUNNING)) {
    throw std::runtime_error("CAN interface \"" + interface_name + "\" is UP but not RUNNING (no carrier)");
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

  _interface_name      = interface_name;
  _communication_error = false;
  return true;
}

bool SocketCANFD::send(ComEndpoint target, const std::vector<uint8_t>& data) {

  canid_t id = mapEndpointToId(target);
  return send(id, data);
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

  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Starting can listener on interface " + _interface_name);

  _listener_is_running = true;
  while (!_shut_down_listener) {
    FD_ZERO(&readSet);

    {
      LockGuard guard(_mutex);
      FD_SET(_soc, &readSet);
      if (select((_soc + 1), &readSet, NULL, NULL, &timeout) >= 0) {
        if (FD_ISSET(_soc, &readSet)) {
          recvbytes = read(_soc, &frame_rd, sizeof(canfd_frame));
          if (recvbytes) {
            try {
              auto endpoint = mapIdToEndpoint(frame_rd.can_id);
              for (const auto& observer : _observers) {
                if (observer)
                  observer->forwardNotification(endpoint, std::vector<std::uint8_t>(frame_rd.data, frame_rd.data + frame_rd.len));
              }
            } catch (const std::exception&) {
              logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Tried to map unknown CAN ID on interface " + _interface_name);
            }
          }
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Stopping can listener on interface " + _interface_name);

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

  if (openInterface(_interface_name)) {
    if (startListener())
      return true;
  }
  return false;
}

void SocketCANFD::fillEndpointMap() {
  canid_t canid_tof_status, canid_tof_request, canid_broadcast;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast);

  canid_t canid_thermal_status, canid_thermal_request, canid_thermal_broadcast;
  CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_STATUS, canid_thermal_status, canid_thermal_request, canid_thermal_broadcast);

  canid_t canid_light_in, canid_light_out, canid_light;
  CanProtocol::makeCanStdID(SYSID_LIGHT, NODEID_HEADLEFT, canid_light_in, canid_light_out, canid_light);

  _id_map[ComEndpoint("tof_status")]      = canid_tof_status;
  _id_map[ComEndpoint("tof_request")]     = canid_tof_request;
  _id_map[ComEndpoint("thermal_status")]  = canid_thermal_status;
  _id_map[ComEndpoint("thermal_request")] = canid_thermal_request;
  _id_map[ComEndpoint("light")]           = canid_light;
  _id_map[ComEndpoint("broadcast")]       = canid_broadcast;
}

void SocketCANFD::addToFSensorToEndpointMap(std::size_t idx) {
  CanProtocol::canid canid_tof_data_in, canid_tof_data_out, canid_broadcast;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_DATA, canid_tof_data_in, canid_tof_data_out, canid_broadcast);

  if ((canid_tof_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Sensor index +" + std::to_string(idx) + " results in a CAN address that is outside the numeric limits of CAN addresses.");
  }

  auto value                  = "tof" + std::to_string(idx) + "_data";
  _id_map[ComEndpoint(value)] = static_cast<CanProtocol::canid>(canid_tof_data_in + idx);
  _endpoints.emplace(value);
}

void SocketCANFD::addThermalSensorToEndpointMap(std::size_t idx) {
  CanProtocol::canid canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast;
  CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_DATA, canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast);

  if ((canid_thermal_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Sensor index +" + std::to_string(idx) + " results in a CAN address that is outside the numeric limits of CAN addresses.");
  }

  auto value                  = "thermal" + std::to_string(idx) + "_data";
  _id_map[ComEndpoint(value)] = static_cast<CanProtocol::canid>(canid_thermal_data_in + idx);
  _endpoints.emplace(value);
}

CanProtocol::canid SocketCANFD::mapEndpointToId(ComEndpoint endpoint) {
  canid_t id = _id_map.at(endpoint); // may throw out_of_range exception
  return id;
}

ComEndpoint SocketCANFD::mapIdToEndpoint(CanProtocol::canid id) {
  auto it = std::find_if(_id_map.begin(), _id_map.end(), [&id](const auto& pair) {
    return pair.second == id;
  });

  if (it != _id_map.end()) {
    return it->first;
  } else {
    throw std::runtime_error("No Endpoint found for given CAN ID");
  }
}

} // namespace com

} // namespace eduart