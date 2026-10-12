// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   PointCloud.hpp
 * @author EduArt Robotik GmbH
 * @brief  Generic point and point cloud types
 * @date   2025-11-20
 */

#pragma once

#include <vector>

#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

/**
 * @class  PointData
 * @brief  Stores the data of a single point in 3d space
 */
struct SENSORRING_EXPORT PointData {
  /// Cartesian coordinates of the point
  math::Vector3 point = { 0.0, 0.0, 0.0 };

  /// Raw distance from the origin to the point
  double raw_distance = 0.0;

  /// Standard deviation associated with the points location
  double sigma = 0.0;

  /// Index of the sensor that measured the point
  unsigned int sensor_index = 0;
};

/**
 * @class  PointCloud
 * @brief  Stores a vector of points in 3D space
 */
struct SENSORRING_EXPORT PointCloud {
  /// Data structure of the PointCloud
  std::vector<PointData> data;

  /**
   * @brief Copies the point cloud to a double buffer
   * @param[in] buffer Pointer to the double buffer. Make sure it has sufficient size.
   * @param[in] size Actual size of the buffer passed to the method. If the buffer is smaller than the point cloud only a subset of points is copied.
   */
  void copyTo(double* buffer, int size);

  /**
   * @brief Combine multiple point clouds into a single cloud.
   * @param[in] clouds Vector of point clouds to merge.
   * @return Single PointCloud containing all points.
   */
  static PointCloud combine(const std::vector<PointCloud>& clouds);

  /**
   * @brief Transforms a point cloud using a rotation matrix and a translation vector.
   * @param[in] cloud The point cloud to transform.
   * @param[in] translation The translation vector.
   * @param[in] rotation The rotation vector (Euler angles in degrees).
   * @return Transformed point cloud.
   */
  static PointCloud transform(const PointCloud& cloud, const math::Vector3 translation, const math::Vector3 rotation);

  /**
   * @brief Transforms a point cloud using a rotation matrix and a translation vector.
   * @param[in] cloud The point cloud to transform.
   * @param[in] translation The translation vector.
   * @param[in] rotation The rotation matrix.
   * @return Transformed point cloud.
   */
  static PointCloud transform(const PointCloud& cloud, const math::Vector3 translation, const math::Matrix3 rotation);
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart