// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DeviceParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for a device.
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/device/DeviceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {
/**
 * @struct DeviceParams
 * @brief Base parameter structure of a device. A device is one sensor or actuator on a sensor board.
 */
struct SENSORRING_EXPORT DeviceParams {
  /// Device identifier (type, name, index).
  DeviceID id;

  /// Whether the device is enabled at creation.
  bool enable;
};

} // namespace device

} // namespace eduart