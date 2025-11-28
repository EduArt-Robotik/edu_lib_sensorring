// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   TofMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Structures for ToF measurements
 * @date   2025-11-20
 */

#pragma once

#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/PointCloud.hpp"

namespace eduart {

namespace measurement {

/**
 * @class  TofMeasurement
 * @brief  Structure for holding a measurement form a ToF sensor
 */
struct SENSORRING_API TofMeasurement {
  unsigned int frame_id = 0;
  PointCloud point_cloud;
};

} // namespace measurement

} // namespace eduart