// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DeviceType.hpp
 * @author EduArt Robotik GmbH
 * @brief  Enumeration of supported device hardware types.
 * @date   2025-02-16
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @enum DeviceType
 * @brief Supported device hardware types used for device registration and grouping.
 */
enum class DeviceType {
  /// Base board (for reset, firmware revision, etc.).
  SensorBoard,
  /// Time-of-flight sensor (VL53L8CX).
  VL53L8CX,
  /// Thermal sensor (HTPA32).
  HTPA32,
  /// LED strip (WS2812b).
  WS2812b,
  /// Time-of-flight sensor (TMF8829).
  TMF8829,
  /// Undefined device type.
  Undefined,
  /// Category wildcard: matches any depth sensor (VL53L8CX, TMF8829, ...).
  AnyDepth,
  /// Category wildcard: matches any thermal sensor (HTPA32, ...).
  AnyThermal,
  /// Category wildcard: matches any light device (WS2812b, ...).
  AnyLight
};

/**
 * @brief Format device type as string.
 * @return Device type string.
 */
SENSORRING_EXPORT std::string toString(DeviceType type) noexcept;

/**
 * @brief Returns true if type is a category wildcard (AnyDepth, AnyThermal, AnyLight).
 */
SENSORRING_EXPORT bool isCategory(DeviceType type) noexcept;

/**
 * @brief Returns true if actual matches expected exactly, or if expected is a category
 *        wildcard and actual belongs to that category.
 */
SENSORRING_EXPORT bool deviceMatchesExpected(DeviceType actual, DeviceType expected) noexcept;

/**
 * @brief Stream device type as string.
 * @param[in] os Output stream.
 * @param[in] type Device type to print.
 * @return Reference to os.
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, const DeviceType type) noexcept;

} // namespace device

} // namespace sensorring

} // namespace eduart