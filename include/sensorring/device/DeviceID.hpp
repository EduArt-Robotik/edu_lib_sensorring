// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   DeviceID.hpp
 * @author EduArt Robotik GmbH
 * @brief  Device type enumeration and identifier (type, name, index) for device registration.
 * @date   2025-02-06
 */

#pragma once

#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/** @brief Supported device hardware types. */
enum class DeviceType {
  VL53L8CX,
  HTPA32,
  WS2812b,
  UNDEFINED
};

/**
 * @class DeviceID
 * @brief Identifier for a device: type, human-readable name, and index.
 */
struct SENSORRING_EXPORT DeviceID {
public:
  /// Hardware device type.
  DeviceType type = DeviceType::UNDEFINED;

  /// Human-readable device name.
  std::string name = "";

  /// Instance index when multiple devices of same type exist.
  unsigned int index = 0;

  /**
   * @brief True if type is not UNDEFINED and name is non-empty.
   * @return true if this ID represents a valid device.
   */
  bool isValid() const;
};

inline bool DeviceID::isValid() const {
  return type != DeviceType::UNDEFINED && name != "";
}

} // namespace device

} // namespace eduart