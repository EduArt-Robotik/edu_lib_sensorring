/**
 * @file   MessageAssembler.hpp
 * @brief  MessageAssembler: fragments a message into TransportFrames.
 */

#pragma once

#include <cstdint>
#include <vector>

#include "sensorring_transport/Protocol.hpp"

namespace eduart {
namespace sensorring {
namespace transport {

/**
 * @class MessageAssembler
 * @brief Splits an outgoing message into one or more TransportFrames.
 *
 * The maximum payload per frame is set at construction time and depends on
 * the underlying transport (e.g. CAN FD: 61 bytes, USB: larger).
 */
class MessageAssembler {
public:
  /**
   * @param maxPayloadPerFrame  Maximum data bytes per frame (excluding protocol header).
   */
  explicit MessageAssembler(std::uint16_t maxPayloadPerFrame);

  /**
   * @brief Fragment a message into one or more TransportFrames.
   *
   * @param direction    Communication direction.
   * @param boardAddress Target board address.
   * @param deviceId     Device byte.
   * @param command      Command byte.
   * @param data         Message payload to fragment.
   * @return Vector of TransportFrames ready for transport encoding.
   */
  std::vector<TransportFrame> assemble(Direction direction, std::uint8_t boardAddress, std::uint8_t deviceId, std::uint8_t command, const std::vector<std::uint8_t>& data) const;

private:
  std::uint16_t _maxPayloadPerFrame;
};

} // namespace transport
} // namespace sensorring
} // namespace eduart