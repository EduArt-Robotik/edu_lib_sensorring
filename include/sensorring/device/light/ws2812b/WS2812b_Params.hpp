// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   WS2812b_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the WS2812b LED strip.
 * @date   2026-05-08
 */

#pragma once

#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct WS2812b_Params
 * @brief Parameter structure of the sensor lights of a sensor board. Not all sensor boards have lights.
 */
struct SENSORRING_EXPORT WS2812b_Params : public DeviceParams {
  /// Orientation of the sensor board. Used to to mirror the light animations.
  Orientation orientation = Orientation::None;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
