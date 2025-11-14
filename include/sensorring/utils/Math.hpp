#pragma once

#include <array>
#include <cmath>

#include "SensorringExport.hpp"

namespace eduart {

namespace math {

// Forward declaration
struct Vector3;
struct Matrix3;
struct Quaternion;

//===============================
// Vector3
//===============================

struct SENSORRING_API Vector3 {
  std::array<double, 3> data;

  double& x();
  const double& x() const;

  double& y();
  const double& y() const;

  double& z();
  const double& z() const;

  double& operator[](std::size_t idx);
  const double& operator[](std::size_t idx) const;

  // Vector addition
  Vector3 operator+(const Vector3& other) const;
  Vector3& operator+=(const Vector3& other);

  // Vector subtraction
  Vector3 operator-(const Vector3& other) const;
  Vector3& operator-=(const Vector3& other);

  // Vector-Scalar multiplication
  Vector3 operator*(const double& other) const;
  Vector3& operator*=(const double& other);

  // Vector-Scalar division
  Vector3 operator/(const double& other) const;
  Vector3& operator/=(const double& other);

  // Get the length of the vector
  double abs() const;

  // Calculate euler angles in degrees of a rotation from a rotation matrix
  static const Vector3 eulerDegreesFromRotationMatrix(const Matrix3& rot_m);

  // Calculate euler angles in radians of a rotation from a rotation matrix
  static const Vector3 eulerRadiansFromRotationMatrix(const Matrix3& rot_m);
};

//===============================
// Matrix3
//===============================

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

  // Calculate a rotation matrix from euler angles given in degrees
  static const Matrix3 rotMatrixFromEulerDegrees(const Vector3& rotation_deg);

  // Calculate a rotation matrix from euler angles given in radians
  static const Matrix3 rotMatrixFromEulerRadians(const Vector3& rotation_rad);
};

} // namespace math

} // namespace eduart