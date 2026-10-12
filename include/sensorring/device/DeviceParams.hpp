// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DeviceParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for a device.
 * @date   2026-02-19
 */

#pragma once

#include <memory>
#include <unordered_map>

#include "sensorring/device/types/DeviceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct DeviceParams
 * @brief Base parameter structure of a device. A device is one sensor or actuator on a sensor board.
 */
struct SENSORRING_EXPORT DeviceParams {
  virtual ~DeviceParams() = default;

  /// Device identifier (type, name, index).
  DeviceID id;

  /// Whether the device is enabled at creation.
  bool enable = true;

  /// Maximum measurement rate this device can sustain (Hz, tenths precision).
  /// Used by the scheduler to compute per-group divisors.
  double max_rate_hz = 15.0;
};

/// Container for device parameters keyed by device type.
using DeviceParamsMap = std::unordered_map<DeviceType, std::shared_ptr<DeviceParams> >;

} // namespace device

} // namespace sensorring

} // namespace eduart