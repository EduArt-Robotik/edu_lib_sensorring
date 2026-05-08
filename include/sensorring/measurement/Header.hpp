// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Header.hpp
 * @author EduArt Robotik GmbH
 * @brief  Universal header for all measurements.
 */

#pragma once

#include "sensorring/device/types/DeviceID.hpp"
#include "sensorring/device/types/DeviceState.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

struct SENSORRING_EXPORT Header {

  /// ID of the sensor that produced this measurement.
  device::DeviceID device_id;

  /// Frame sequence counter.
  unsigned int frame_id = 0;

  /// Timestamp when the measurement was taken.
  std::chrono::system_clock::time_point timestamp;

  /// Device health state at the time of publication.
  device::DeviceState state = device::DeviceState::Undefined;

  /// Sensor pose translation
  math::Vector3 position = { 0.0, 0.0, 0.0 };

  // Sensor pose orientation
  math::Vector3 orientation = { 0.0, 0.0, 0.0 };
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart