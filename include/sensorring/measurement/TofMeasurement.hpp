// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   TofMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Structures for ToF measurements
 * @date   2025-11-20
 */

#pragma once

#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/PointCloud.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

/**
 * @class  TofMeasurement
 * @brief  Structure for holding a measurement from a ToF sensor
 */
struct TofMeasurement {
  /// Frame number of the TofMeasurement
  unsigned int frame_id = 0;

  /// Number of valid points in the TofMeasurement
  unsigned int nr_valid_points = 0;

  /// Point cloud of the Time-of-Flight sensor measurement
  PointCloud point_cloud;
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart