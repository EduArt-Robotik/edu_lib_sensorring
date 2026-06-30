// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   VL53L8CX_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the VL53L8CX Time-of-Flight sensor.
 * @date   2026-05-08
 */

#pragma once

#include "sensorring/device/depth/DepthSensorParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct VL53L8CX_Params
 * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
 */
struct SENSORRING_EXPORT VL53L8CX_Params : public DepthSensorParams {

  /// @brief Default constructor; sets the maximum measurement rate to 15 Hz.
  VL53L8CX_Params() { max_rate_hz = 15.0; }

  /// @brief Initializes the VL53L8CX_Params from a DepthSensorParams instance.
  VL53L8CX_Params(const DepthSensorParams& params)
      : DepthSensorParams{ params } {
    max_rate_hz = 15.0;
  };
};

} // namespace device

} // namespace sensorring

} // namespace eduart
