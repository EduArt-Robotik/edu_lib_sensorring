// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Pure data structure for depth sensor measurements.
 */

#pragma once

#include <chrono>
#include <vector>

#include "sensorring/device/DeviceState.hpp"
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
  /// Index of the sensor that produced this measurement.
  unsigned int sensor_index = 0;

  /// Frame sequence counter.
  unsigned int frame_id = 0;

  /// Number of valid points in this measurement.
  unsigned int nr_valid_points = 0;

  /// Timestamp when the measurement was taken.
  std::chrono::system_clock::time_point timestamp;

  /// Device health state at the time of publication.
  device::DeviceState state = device::DeviceState::Undefined;

  /// Point cloud in the sensor's local coordinate frame.
  PointCloud point_cloud;

  /// Point cloud transformed into the ring's coordinate frame (using configured pose).
  PointCloud transformed_point_cloud;

  /**
   * @brief Combine the transformed point clouds from multiple depth measurements.
   * @param[in] measurements Vector of depth measurements to merge.
   * @return Single PointCloud containing all transformed points.
   */
  static PointCloud combinePointClouds(const std::vector<DepthMeasurement>& measurements);
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart
