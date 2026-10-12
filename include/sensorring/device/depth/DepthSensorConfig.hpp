// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthSensorConfig.hpp
 * @author EduArt Robotik GmbH
 * @brief  Configuration options for depth-sensing devices (ToF, structured light, etc.).
 * @date   2026-06-22
 */

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @brief Configuration structure for 3D depth sensors.
 * @todo  May be combined with the DepthSensorParams.
 */
struct SENSORRING_EXPORT DepthSensorConfig {
  /// Horizontal field of view in degrees.
  double fov_x_deg = 0.0;

  /// Vertical field of view in degrees.
  double fov_y_deg = 0.0;

  /// Horizontal resolution in pixels (columns).
  unsigned int res_x = 0;

  /// Vertical resolution in pixels (rows).
  unsigned int res_y = 0;

  /// Invert x LUT to match sensor coordinate system
  bool invert_x_lut = false;

  /// Invert y LUT to match sensor coordinate system
  bool invert_y_lut = false;

  /// Apply correction factor to convert from direct distance (hypotenuse) to perpendicular distance if the sensor does not report perpendicular distance directly.
  bool reports_perpendicular_distance = true;
};

} // namespace device

} // namespace sensorring

} // namespace eduart