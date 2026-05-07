// Copyright (c) 2026 EduArt Robotik GmbH
//
// Parameter structure for the VL53L8CX Time-of-Flight sensor.

#pragma once

#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct VL53L8CX_Params
 * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
 */
struct SENSORRING_EXPORT VL53L8CX_Params : public DeviceParams {
  VL53L8CX_Params() { max_rate_hz = 15.0; }
};

} // namespace device

} // namespace sensorring

} // namespace eduart
