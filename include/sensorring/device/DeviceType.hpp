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

namespace device {

/**
 * @enum DeviceType
 * @brief Supported device hardware types used for device registration and grouping.
 */
enum class DeviceType {
  /// Time-of-flight sensor (VL53L8CX).
  VL53L8CX,
  /// Thermal sensor (HTPA32).
  HTPA32,
  /// LED strip (WS2812b).
  WS2812b,
  /// Undefined device type.
  UNDEFINED
};

} // namespace device

} // namespace eduart