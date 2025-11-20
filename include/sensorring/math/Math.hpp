// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Math.hpp
 * @author EduArt Robotik GmbH
 * @brief  Collection of math functions
 * @date   2025-11-20
 */

#pragma once

#include "sensorring/math/Matrix3.hpp"
#include "sensorring/math/Vector3.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace math {

// Calculate euler angles in degrees of a rotation from a rotation matrix
SENSORRING_API const Vector3 eulerDegreesFromRotationMatrix(const Matrix3& rot_m);

// Calculate euler angles in radians of a rotation from a rotation matrix
SENSORRING_API const Vector3 eulerRadiansFromRotationMatrix(const Matrix3& rot_m);

// Calculate a rotation matrix from euler angles given in degrees
SENSORRING_API const Matrix3 rotMatrixFromEulerDegrees(const Vector3& rotation_deg);

// Calculate a rotation matrix from euler angles given in radians
SENSORRING_API const Matrix3 rotMatrixFromEulerRadians(const Vector3& rotation_rad);

} // namespace math

} // namespace eduart