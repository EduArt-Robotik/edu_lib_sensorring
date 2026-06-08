// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Pure data structure for depth sensor measurements.
 * @date   2026-05-08
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

  /// Resolution of the depth sensor in x direction. Part of the DepthMeasurement as the PointCloud itself may be unstructured.
  unsigned int resolution_x = 0;

  /// Resolution of the depth sensor in y direction. Part of the DepthMeasurement as the PointCloud itself may be unstructured.
  unsigned int resolution_y = 0;

  /// Point cloud in the sensor's local coordinate frame.
  PointCloud point_cloud;

  /**
   * @brief Transform the point cloud from the sensor's local frame to the ring's global frame.
   * @return A new PointCloud with all points expressed in the global coordinate frame.
   */
  inline PointCloud transformToGlobalFrame() const { return PointCloud::transform(point_cloud, header.position, header.orientation); }
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart
