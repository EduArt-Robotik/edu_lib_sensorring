// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   WS2812b_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the WS2812b LED strip.
 * @date   2026-05-08
 */

#pragma once

#include "sensorring/device/light/LightParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct WS2812b_Params
 * @brief Parameter structure of the WS2812b LEDs.
 */
struct SENSORRING_EXPORT WS2812b_Params : public LightParams {

  /// @brief Default constructor.
  WS2812b_Params() { max_rate_hz = 15.0; }

  /// @brief Initializes the WS2812b_Params from a LightParams instance.
  WS2812b_Params(const LightParams& params)
      : LightParams{ params } {
    max_rate_hz = 15.0;
  };
};

} // namespace device

} // namespace sensorring

} // namespace eduart
