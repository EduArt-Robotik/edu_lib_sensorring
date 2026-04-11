// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ComEndpoint.hpp
 * @author EduArt Robotik GmbH
 * @brief  Communication endpoint identifier.
 * @date   2026-02-19
 */

#pragma once

#include <cstdint>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @enum Direction
 * @brief Communication direction on the CAN bus.
 */
enum class Direction : std::uint8_t {
  Input  = 0, ///< Board -> host
  Output = 1  ///< Host -> board
};

/**
 * @struct ComEndpoint
 * @brief Uniquely identifies a communication endpoint as a (Direction, BoardAddress, DeviceId) tuple.
 */
struct SENSORRING_EXPORT ComEndpoint {
  Direction direction;       ///< board->host (Input) or host->board (Output)
  std::uint8_t boardAddress; ///< 0 = broadcast, 1-126 = individual board (board_index + 1)
  std::uint8_t deviceId;     ///< 0x00 = board, 0x01 = ToF, 0x02 = Thermal, 0x03 = WS2812b

  bool operator==(const ComEndpoint& other) const;
  bool operator!=(const ComEndpoint& other) const;

  /**
   * @brief Human-readable representation for logging.
   * @return e.g. "Input/Board5/VL53L8CX", "Output/Broadcast/Board", "Input/ANY/Board"
   */
  std::string toString() const;

  /// Broadcast board address (node ID 0x00 per existing CAN convention).
  static constexpr std::uint8_t BROADCAST = 0x00;

  /// Subscription wildcard: matches any board address during dispatch.
  static constexpr std::uint8_t ANY_BOARD = 0xFF;
};

/**
 * @brief Check whether a subscription endpoint matches an incoming endpoint.
 *
 * Supports the ANY_BOARD wildcard: if subscription.boardAddress == ANY_BOARD,
 * it matches any incoming board address.
 */
inline bool endpointMatches(const ComEndpoint& subscription, const ComEndpoint& incoming) {
  return subscription.direction == incoming.direction && (subscription.boardAddress == incoming.boardAddress || subscription.boardAddress == ComEndpoint::ANY_BOARD) && subscription.deviceId == incoming.deviceId;
}

} // namespace com

} // namespace sensorring

} // namespace eduart

namespace std {

template <> struct hash<eduart::sensorring::com::ComEndpoint> {
  std::size_t operator()(const eduart::sensorring::com::ComEndpoint& ep) const {
    std::uint32_t packed = (static_cast<std::uint32_t>(ep.direction) << 16) | (static_cast<std::uint32_t>(ep.boardAddress) << 8) | static_cast<std::uint32_t>(ep.deviceId);
    return std::hash<std::uint32_t>{}(packed);
  }
};

} // namespace std