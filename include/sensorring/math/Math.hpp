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

/**
 * Calculate euler angles of a rotation from a rotation matrix (RPY-convention)
 * @param[in] rot_m Rotation matrix to be converted to euler angles
 * @return Euler angles in degrees
 */
SENSORRING_EXPORT const Vector3 eulerDegreesFromRotationMatrix(const Matrix3& rot_m);

/**
 * Calculate euler angles of a rotation from a rotation matrix (RPY-convention)
 * @param[in] rot_m Rotation matrix to be converted to euler angles
 * @return Euler angles in radians
 */
SENSORRING_EXPORT const Vector3 eulerRadiansFromRotationMatrix(const Matrix3& rot_m);

/**
 * Calculate a rotation matrix from euler angles (RPY-convention)
 * @param[in] rotation_deg Euler angles in degrees to be converted to a rotation matrix
 * @return Rotation matrix
 */
SENSORRING_EXPORT const Matrix3 rotMatrixFromEulerDegrees(const Vector3& rotation_deg);

/**
 * Calculate a rotation matrix from euler angles (RPY-convention)
 * @param[in] rotation_deg Euler angles in radians to be converted to a rotation matrix
 * @return Rotation matrix
 */
SENSORRING_EXPORT const Matrix3 rotMatrixFromEulerRadians(const Vector3& rotation_rad);

} // namespace math

} // namespace eduart