// Copyright (c) 2026 EduArt Robotik GmbH
//
// Parameter structure for the VL53L8CX Time-of-Flight sensor.

#pragma once

#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @struct VL53L8CX_Params
 * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
 */
struct SENSORRING_EXPORT VL53L8CX_Params : public DeviceParams {};

} // namespace device

} // namespace eduart
