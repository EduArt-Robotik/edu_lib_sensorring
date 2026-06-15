// Copyright (c) 2026 EduArt Robotik GmbH

#include "board/SensorBoardCommands.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace board {

bool resetBoards() {
  bool success = true;
  for (auto& iface : com::ComManager::getInstance()->getInterfaces()) {
    success &= iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::BOARD }, sensor_board::RESET, {});
  }
  return success;
}

void cmdEnumerateBoards(com::ComInterfaceID interface) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (iface) {
    iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::BOARD }, sensor_board::ACTIVE_DEVICE_REQUEST, {});
  }
}

} // namespace board

} // namespace sensorring

} // namespace eduart
