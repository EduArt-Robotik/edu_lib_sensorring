#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <sensorring_transport/can/CanCodec.hpp>

namespace eduart {
namespace sensorring {
namespace com {
namespace ethernet {

constexpr std::size_t DATAGRAM_HEADER_SIZE = 5;

inline std::vector<std::uint8_t> encodeDatagram(
    std::uint8_t bus_index, std::uint32_t can_id, const std::vector<std::uint8_t>& can_data) {
  if (can_data.size() > transport::can::CanCodec::CAN_FD_MAX_DLEN) {
    return {};
  }

  std::vector<std::uint8_t> datagram(DATAGRAM_HEADER_SIZE + can_data.size() + 4);
  datagram[0] = 0xFF; // ETHERNET_START_BYTE
  datagram[1] = 0x00; // Sequence Nbr
  datagram[2] = 0x05; // ETHERNET_GET_SENSORRING
  datagram[3] = bus_index;
  datagram[4] = static_cast<std::uint8_t>((can_id >> 24) & 0xFF);
  datagram[5] = static_cast<std::uint8_t>((can_id >> 16) & 0xFF);
  datagram[6] = static_cast<std::uint8_t>((can_id >> 8) & 0xFF);
  datagram[7] = static_cast<std::uint8_t>(can_id & 0xFF);
  for (std::size_t i = 0; i < can_data.size(); ++i) {
    datagram[DATAGRAM_HEADER_SIZE + 3 + i] = can_data[i];
  }
  datagram.back() = 0xEE; // ETHERNET_END_BYTE
  return datagram;
}

inline bool decodeDatagram(
    const std::uint8_t* datagram, std::size_t datagram_size, std::uint8_t expected_bus_index, std::uint32_t& can_id, std::vector<std::uint8_t>& can_data) {
  if (!datagram || datagram_size < DATAGRAM_HEADER_SIZE + 3
      || datagram_size > DATAGRAM_HEADER_SIZE + transport::can::CanCodec::CAN_FD_MAX_DLEN
      || datagram[0] != expected_bus_index) {
    return false;
  }

  can_id = (static_cast<std::uint32_t>(datagram[1]) << 24)
      | (static_cast<std::uint32_t>(datagram[2]) << 16)
      | (static_cast<std::uint32_t>(datagram[3]) << 8)
      | static_cast<std::uint32_t>(datagram[4]);
  can_data.assign(datagram + DATAGRAM_HEADER_SIZE, datagram + datagram_size);
  return true;
}

} // namespace ethernet
} // namespace com
} // namespace sensorring
} // namespace eduart
