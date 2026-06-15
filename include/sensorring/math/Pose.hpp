// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Pose.hpp
 * @author EduArt Robotik GmbH
 * @brief  Defines a pose with translation and orientation.
 * @date   2025-02-09
 */

#pragma once

#include "sensorring/math/Vector3.hpp"

namespace eduart {

namespace sensorring {

namespace math {

/**
 * @struct Pose
 * @brief Defines a pose with translation and orientation.
 */
struct Pose {
  /// Translation (x, y, z).
  math::Vector3 translation;

  /// Orientation (roll, pitch, yaw).
  math::Vector3 orientation;

  /// Addition operator to combine two poses.
  Pose operator+(const Pose& other) const;

  /// Subtraction operator to find the difference between two poses.
  Pose operator-(const Pose& other) const;
};

} // namespace math
} // namespace sensorring

} // namespace eduart
