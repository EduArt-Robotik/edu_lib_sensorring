// Copyright (c) 2026 EduArt Robotik GmbH

#include "SensorBoardCommands.hpp"

#include "interface/ComManager.hpp"
#include <sensorring_transport/Protocol.hpp>
using namespace eduart::transport::protocol;

namespace eduart {

namespace device {

bool resetBoards() {
  bool success = true;
  for (auto& iface : com::ComManager::getInstance()->getInterfaces()) {
    success &= iface->send(com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::BROADCAST, devbyte::BOARD }, sensor_board::RESET, {});
  }
  return success;
}

void cmdSetBitRateSwitching(com::ComInterfaceID interface, bool enable) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (iface) {
    // TODO: BRS command removed in v2 protocol — re-add when firmware supports it.
    (void)enable;
  }
}

void cmdEnumerateBoards(com::ComInterfaceID interface) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (iface) {
    iface->send(com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::BROADCAST, devbyte::BOARD }, sensor_board::ACTIVE_DEVICE_REQUEST, {});
  }
}

} // namespace device

} // namespace eduart
