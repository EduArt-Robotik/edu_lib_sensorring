// Copyright (c) 2026 EduArt Robotik GmbH
//
// Parameter structure for the TMF8829 Time-of-Flight sensor.

#pragma once

#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct TMF8829_Params
 * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
 */
struct SENSORRING_EXPORT TMF8829_Params : public DeviceParams {
  TMF8829_Params() { max_rate_hz = 15.0; }
};

} // namespace device

} // namespace sensorring

} // namespace eduart
