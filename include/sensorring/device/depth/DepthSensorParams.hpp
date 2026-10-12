// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthSensorParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the DepthSensor.
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct DepthSensorParams
 * @brief Parameter structure of the DepthSensor.
 */
struct SENSORRING_EXPORT DepthSensorParams : public DeviceParams {
  /// Destructor
  virtual ~DepthSensorParams() = default;
};

} // namespace device

} // namespace sensorring

} // namespace eduart