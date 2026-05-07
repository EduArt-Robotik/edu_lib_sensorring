// Copyright (c) 2026 EduArt Robotik GmbH

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

namespace sensorring {

namespace math {

/// PI constant
static constexpr double PI = 3.141592653589793238462643383279502884;

/**
 * @brief Convert degrees to radians.
 */
SENSORRING_EXPORT double degreesToRadians(double degrees);

/**
 * @brief Convert radians to degrees.
 */
SENSORRING_EXPORT double radiansToDegrees(double radians);

/**
 * @brief Calculate euler angles of a rotation from a rotation matrix (RPY-convention).
 * @param[in] rot_m Rotation matrix to be converted to euler angles
 * @return Euler angles in degrees
 */
SENSORRING_EXPORT const Vector3 eulerDegreesFromRotationMatrix(const Matrix3& rot_m);

/**
 * @brief Calculate euler angles of a rotation from a rotation matrix (RPY-convention).
 * @param[in] rot_m Rotation matrix to be converted to euler angles
 * @return Euler angles in radians
 */
SENSORRING_EXPORT const Vector3 eulerRadiansFromRotationMatrix(const Matrix3& rot_m);

/**
 * @brief Calculate a rotation matrix from euler angles (RPY-convention).
 * @param[in] rotation_deg Euler angles in degrees to be converted to a rotation matrix
 * @return Rotation matrix
 */
SENSORRING_EXPORT const Matrix3 rotMatrixFromEulerDegrees(const Vector3& rotation_deg);

/**
 * @brief Calculate a rotation matrix from euler angles (RPY-convention).
 * @param[in] rotation_rad Euler angles in radians to be converted to a rotation matrix
 * @return Rotation matrix
 */
SENSORRING_EXPORT const Matrix3 rotMatrixFromEulerRadians(const Vector3& rotation_rad);

} // namespace math

} // namespace sensorring

} // namespace eduart