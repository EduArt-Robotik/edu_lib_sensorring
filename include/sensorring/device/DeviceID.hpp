// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   DeviceID.hpp
 * @author EduArt Robotik GmbH
 * @brief  Device type enumeration and identifier (type, name, index) for device registration.
 * @date   2025-02-06
 */

#pragma once

#include <string>

#include "sensorring/device/DeviceType.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

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
   * @brief Returns the hardware device type.
   * @return DeviceType of this ID.
   */
  DeviceType getType() const;

  /**
   * @brief Returns the human-readable device name.
   * @return Name string.
   */
  std::string getName() const;

  /**
   * @brief Returns the instance index when multiple devices of the same type exist.
   * @return Index value.
   */
  unsigned int getIndex() const;

  /**
   * @brief Check if the device ID belongs to a valid device.
   * @return true if this ID represents a valid device.
   */
  bool isValid() const;
};

inline DeviceType DeviceID::getType() const {
  return type;
}

inline std::string DeviceID::getName() const {
  return name;
}

inline unsigned int DeviceID::getIndex() const {
  return index;
}

inline bool DeviceID::isValid() const {
  return type != DeviceType::UNDEFINED && name != "";
}

} // namespace device

} // namespace eduart