/**
 * @file   CanCodec.hpp
 * @brief  CAN FD transport codec: TransportFrame ↔ raw CAN FD frame bytes + CAN ID math.
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "sensorring_transport/Protocol.hpp"

namespace eduart {
namespace sensorring {
namespace transport {
namespace can {

/**
 * @struct CanFrame
 * @brief Raw CAN FD frame representation (ID + data bytes).
 */
struct CanFrame {
  std::uint32_t id;
  std::vector<std::uint8_t> data; ///< Complete frame data including protocol header
};

/**
 * @class CanCodec
 * @brief Encodes/decodes TransportFrames to/from CAN FD wire format.
 *
 * CAN ID layout (11-bit standard):
 *   bits 10-8: system ID (SYSID_SENSOR_RING = 0b110)
 *   bit 7:     direction (0 = input/board→host, 1 = output/host→board)
 *   bits 6-0:  board address (0x00 = broadcast, 0x01-0x7E = individual)
 *
 * CAN FD payload layout (first 3 bytes = protocol header):
 *   byte 0: fragment byte [7:6]=type, [5:0]=count
 *   byte 1: device ID (devbyte::*)
 *   byte 2: command
 *   bytes 3+: payload data
 */
class CanCodec {
public:
  /// Maximum payload data bytes per CAN FD frame (64 - 3 byte header).
  static constexpr std::uint16_t MAX_PAYLOAD_PER_FRAME = 61;

  /// CAN FD maximum frame data length.
  static constexpr std::size_t CAN_FD_MAX_DLEN = 64;

  CanCodec() = delete;

  // ── CAN ID helpers ────────────────────────────────────────────────

  /// System ID for sensor ring protocol.
  static constexpr std::uint32_t SYSID_SENSOR_RING = 0b110;

  /// Build the CAN ID for a given direction and board address.
  static std::uint32_t makeCanId(std::uint8_t direction, std::uint8_t boardAddress);

  /// Build input + output CAN IDs for a board address.
  static void makeCanIds(std::uint8_t boardAddress, std::uint32_t& inputId, std::uint32_t& outputId);

  /// Extract board address from a CAN ID.
  static std::uint8_t extractBoardAddress(std::uint32_t canId);

  /// Extract direction bit from a CAN ID.
  static std::uint8_t extractDirection(std::uint32_t canId);

  // ── Fragment byte helpers ─────────────────────────────────────────

  /// Build the fragment byte from type and count.
  static std::uint8_t makeFragmentByte(std::uint8_t type, std::uint8_t count);

  /// Extract fragment type (bits 7-6) from fragment byte.
  static std::uint8_t fragmentType(std::uint8_t fragmentByte);

  /// Extract count field (bits 5-0) from fragment byte.
  static std::uint8_t fragmentCount(std::uint8_t fragmentByte);

  // ── Encode / Decode ───────────────────────────────────────────────

  /**
   * @brief Encode a TransportFrame into a raw CAN FD CanFrame.
   * @param frame  The TransportFrame to encode.
   * @return CanFrame with CAN ID and data bytes.
   */
  static CanFrame encode(const TransportFrame& frame);

  /**
   * @brief Decode a raw CAN FD frame into a TransportFrame.
   *
   * @param canId    CAN arbitration ID.
   * @param data     Raw CAN FD frame data (including 3-byte protocol header).
   * @param dataLen  Number of valid bytes in the data buffer.
   * @return Decoded TransportFrame.
   */
  static TransportFrame decode(std::uint32_t canId, const std::uint8_t* data, std::size_t dataLen);
};

} // namespace can
} // namespace transport
} // namespace sensorring
} // namespace eduart