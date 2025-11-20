#include "sensorring/math/Matrix3.hpp"

#include <cmath>

namespace eduart {

namespace math {

Vector3& Matrix3::operator[](std::size_t idx) {
  return data[idx];
}

const Vector3& Matrix3::operator[](std::size_t idx) const {
  return data[idx];
}

Matrix3 Matrix3::operator*(const Matrix3& other) const {
  Matrix3 result{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[i][j] = 0.0f;
      for (int k = 0; k < 3; ++k) {
        result[i][j] += data[i][k] * other[k][j];
      }
    }
  }
  return result;
}

Matrix3& Matrix3::operator*=(const Matrix3& other) {
  *this = *this * other;
  return *this;
}

Vector3 Matrix3::operator*(const Vector3& other) const {
  Vector3 result{};
  for (int i = 0; i < 3; ++i) {
    result[i] = 0.0f;
    for (int j = 0; j < 3; ++j) {
      result[i] += data[i][j] * other[j];
    }
  }
  return result;
}

Matrix3 Matrix3::operator*(const double& other) const {
  Matrix3 result{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[i][j] = 0.0f;
      for (int k = 0; k < 3; ++k) {
        result[i][j] += data[i][k] * other;
      }
    }
  }
  return result;
}

Matrix3& Matrix3::operator*=(const double& other) {
  *this = *this * other;
  return *this;
}

Matrix3 Matrix3::operator/(const double& other) const {
  Matrix3 result{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[i][j] = 0.0f;
      for (int k = 0; k < 3; ++k) {
        result[i][j] += data[i][k] / other;
      }
    }
  }
  return result;
}

Matrix3& Matrix3::operator/=(const double& other) {
  *this = *this / other;
  return *this;
}

} // namespace math

} // namespace eduart