// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Matrix3.hpp
 * @author EduArt Robotik GmbH
 * @brief  Implementation of a matrix of size 3 × 3
 * @date   2025-11-20
 */

#pragma once

#include <array>
#include <cmath>

#include "sensorring/math/Vector3.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace math {

/**
 * @class Matrix3
 * @brief Matrix of size 3 × 3
 */
struct SENSORRING_API Matrix3 {
  std::array<Vector3, 3> data;

  Vector3& operator[](std::size_t idx);

  const Vector3& operator[](std::size_t idx) const;

  // Matrix multiplication: Matrix3 * Matrix3 -> Matrix3
  Matrix3 operator*(const Matrix3& other) const;
  Matrix3& operator*=(const Matrix3& other);

  // Matrix-Vector multiplication: Matrix3 * Vector3 -> Vector3
  Vector3 operator*(const Vector3& other) const;

  // Matrix-Scalar multiplication: Matrix3 * double -> Matrix3
  Matrix3 operator*(const double& other) const;
  Matrix3& operator*=(const double& other);

  // Matrix-Scalar division: Matrix3 / double -> Matrix3
  Matrix3 operator/(const double& other) const;
  Matrix3& operator/=(const double& other);
};

} // namespace math

} // namespace eduart