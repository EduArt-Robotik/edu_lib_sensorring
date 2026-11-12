#pragma once

#include <algorithm>
#include <cstdint>
#include <sensorring_transport/can/CanFdDlc.hpp>
#include <stdexcept>
#include <usbtingo/device/DeviceHelper.hpp>
#include <vector>

namespace eduart {
namespace sensorring {
namespace com {

inline usbtingo::device::CanTxFrame makeUsbTingoCanTxFrame(std::uint32_t can_id, const std::vector<std::uint8_t>& data, bool brs) {
  usbtingo::device::CanTxFrame frame{};
  if (data.size() > frame.data.size()) {
    throw std::invalid_argument("USBtingo CAN FD frame exceeds 64 bytes");
  }
  frame.id = can_id;
  // USBtingo's serializer expects the DLC code, not the rounded byte length.
  frame.dlc = transport::can::bytesToDlcCode(data.size());
  frame.fdf = 1;
  frame.brs = brs ? 1 : 0;
  std::copy(data.begin(), data.end(), frame.data.begin());
  return frame;
}

} // namespace com
} // namespace sensorring
} // namespace eduart
