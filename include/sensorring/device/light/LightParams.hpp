// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   LightParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the Light.
 * @date   2026-02-19
 */

#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

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
  /// Lower DistanceMap saturation threshold in meters. Lights are red below this distance.
  double distance_map_lower_m = 0.1;

  /// Upper DistanceMap saturation threshold in meters. Lights are green above this distance.
  double distance_map_upper_m = 0.5;

  /// DistanceMap peak channel brightness on the LED's 0-255 scale (0 disables it).
  std::int64_t distance_map_max_brightness = 50;

  /// Validates the distance map parameters.
  void validateDistanceMapBrightness() const {
    if (distance_map_max_brightness < 0 || distance_map_max_brightness > 255) {
      throw std::invalid_argument("LED distance_map_max_brightness must be an integer in [0, 255].");
    }
  }

  /// Throws if the range cannot be represented by the firmware's float32 parameters.
  void validateDistanceMapRange() const {
    if (!std::isfinite(distance_map_lower_m) || !std::isfinite(distance_map_upper_m) || distance_map_lower_m < 0.0 || distance_map_upper_m <= distance_map_lower_m || distance_map_upper_m > std::numeric_limits<float>::max()
        || static_cast<float>(distance_map_lower_m) >= static_cast<float>(distance_map_upper_m)) {
      throw std::invalid_argument("LED distance_map_lower_m and distance_map_upper_m must be finite meters with 0 <= lower < upper, distinct and representable as float32.");
    }
  }

  /// Destructor
  virtual ~LightParams() = default;
};

} // namespace device

} // namespace sensorring

} // namespace eduart