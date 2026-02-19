// Copyright (c) 2026 EduArt Robotik GmbH
//
// Parameter structure for the WS2812b LED strip.

#pragma once

#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @enum Orientation
 * @brief Possible orientations of a sensor board. Used to rotate/mirror light animations.
 */
enum class Orientation {
  left,
  right,
  none
};

/**
 * @struct WS2812b_Params
 * @brief Parameter structure of the sensor lights of a sensor board. Not all sensor boards have lights.
 */
struct SENSORRING_EXPORT WS2812b_Params : public DeviceParams {
  /// Orientation of the sensor board. Used to to mirror the light animations.
  Orientation orientation = Orientation::none;
};

} // namespace device

} // namespace eduart
