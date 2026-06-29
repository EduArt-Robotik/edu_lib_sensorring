// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   LightParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the Light.
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct LightParams
 * @brief Parameter structure of the Light.
 */
struct SENSORRING_EXPORT LightParams : public DeviceParams {
  /// Destructor
  virtual ~LightParams() = default;

  /// Orientation of the sensor board. Used to to mirror the light animations.
  Orientation orientation = Orientation::None;
};

} // namespace device

} // namespace sensorring

} // namespace eduart