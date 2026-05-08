// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Pure data structure for depth sensor measurements.
 */

#pragma once

#include <chrono>
#include <vector>

#include "sensorring/measurement/Header.hpp"
#include "sensorring/measurement/PointCloud.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

/**
 * @struct DepthMeasurement
 * @brief Pure data structure holding a measurement from a depth sensor.
 *
 * Delivered to subscribers via DepthSensor::subscribe(). Contains both the
 * raw (sensor-frame) and transformed (ring-frame) point clouds.
 */
struct SENSORRING_EXPORT DepthMeasurement {

  /// Measurement header
  Header header;

  /// Number of valid points in this measurement.
  unsigned int nr_valid_points = 0;

  /// Point cloud in the sensor's local coordinate frame.
  PointCloud point_cloud;

  /// Point cloud transformed into the ring's coordinate frame (using configured pose).
  PointCloud transformed_point_cloud;

  /**
   * @brief Combine the transformed point clouds from multiple depth measurements.
   * @param[in] measurements Vector of depth measurements to merge.
   * @return Single PointCloud containing all transformed points.
   */
  static inline PointCloud combinePointClouds(const std::vector<DepthMeasurement>& measurements) {
    std::vector<PointCloud> clouds;
    clouds.reserve(measurements.size());
    for (const auto& m : measurements) {
      clouds.push_back(m.transformed_point_cloud);
    }
    return PointCloud::combine(clouds);
  }
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart
