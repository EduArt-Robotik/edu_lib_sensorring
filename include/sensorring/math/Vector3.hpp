// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Vector3.hpp
 * @author EduArt Robotik GmbH
 * @brief  Implementation of a vector with length 3
 * @date   2025-11-20
 */

#pragma once

#include <array>
#include <cmath>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace math {

/**
 * @class Vector3
 * @brief Vector of length 3
 */
struct SENSORRING_API Vector3 {
  /// Data structure of the Vector 3
  std::array<double, 3> data;

  /// First component of the vector
  double& x();

  /// First component of the vector
  const double& x() const;

  /// Second component of the vector
  double& y();

  /// Second component of the vector
  const double& y() const;

  /// Third component of the vector
  double& z();

  /// Third component of the vector
  const double& z() const;

  /// Indexing the Matrix
  double& operator[](std::size_t idx);

  /// Indexing the Matrix
  const double& operator[](std::size_t idx) const;

  /// Vector addition
  Vector3 operator+(const Vector3& other) const;

  /// Vector addition
  Vector3& operator+=(const Vector3& other);

  /// Vector subtraction
  Vector3 operator-(const Vector3& other) const;

  /// Vector subtraction
  Vector3& operator-=(const Vector3& other);

  /// Vector-Scalar multiplication
  Vector3 operator*(const double& other) const;

  /// Vector-Scalar multiplication
  Vector3& operator*=(const double& other);

  /// Vector-Scalar division
  Vector3 operator/(const double& other) const;

  /// Vector-Scalar division
  Vector3& operator/=(const double& other);

  /// Get the length of the vector
  double abs() const;
};

} // namespace math

} // namespace eduart