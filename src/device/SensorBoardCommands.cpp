// Copyright (c) 2026 EduArt Robotik GmbH

#include "SensorBoardCommands.hpp"

#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace device {

bool resetBoards() {
  bool success                = true;
  std::vector<uint8_t> tx_buf = { CMD_HARD_RESET };
  for (auto& iface : com::ComManager::getInstance()->getInterfaces()) {
    success &= iface->send(com::ComEndpoint("broadcast"), tx_buf);
  }
  return success;
}

void cmdSetBitRateSwitching(com::ComInterfaceID interface, bool enable) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (iface) {
    std::vector<uint8_t> tx_buf = { CMD_SET_BRS, 0xFF, 0xFF, enable ? std::uint8_t(0x01) : std::uint8_t(0x00) };
    iface->send(com::ComEndpoint("broadcast"), tx_buf);
  }
}

void cmdEnumerateBoards(com::ComInterfaceID interface) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (iface) {
    std::vector<uint8_t> tx_buf_enumeration = { CMD_ACTIVE_DEVICE_QUERY, CMD_ACTIVE_DEVICE_QUERY };
    iface->send(com::ComEndpoint("broadcast"), tx_buf_enumeration);
  }
}

} // namespace device

} // namespace eduart
