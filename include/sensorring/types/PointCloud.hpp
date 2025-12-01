// Copyright (c) 2025 EduArt Robotik GmbH

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

namespace measurement {

/**
 * @class  PointData
 * @brief  Stores the data of a single point in 3d space
 */
struct SENSORRING_API PointData {
  math::Vector3 point = { 0.0, 0.0, 0.0 };
  double raw_distance = 0.0;
  double sigma        = 0.0;
  int user_idx        = 0;
};

/**
 * @class  PointCloud
 * @brief  Stores a vector of points in 3D space
 */
struct SENSORRING_API PointCloud {
  std::vector<PointData> data;

  /**
   * @brief Copies the point cloud to a double buffer
   * @param[in] buffer Pointer to the double buffer. Make sure it has sufficient size.
   * @param[in] size Actual size of the buffer passed to the method. If the buffer is smaller than the point cloud only a subset of points is copied.
   */
  void copyTo(double* buffer, int size);
};

} // namespace measurement

} // namespace eduart