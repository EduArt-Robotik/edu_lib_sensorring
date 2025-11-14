#include "sensorring/utils/Math.hpp"

namespace eduart {

namespace math {

//===============================
// Vector3
//===============================

double& Vector3::x() {
  return data[0];
}
const double& Vector3::x() const {
  return data[0];
}

double& Vector3::y() {
  return data[1];
}
const double& Vector3::y() const {
  return data[1];
}

double& Vector3::z() {
  return data[2];
}
const double& Vector3::z() const {
  return data[2];
}

double& Vector3::operator[](std::size_t idx) {
  return data[idx];
}
const double& Vector3::operator[](std::size_t idx) const {
  return data[idx];
}

Vector3 Vector3::operator+(const Vector3& other) const {
  Vector3 result{};
  for (int i = 0; i < 3; ++i) {
    result[i] = data[i] + other[i];
  }
  return result;
}

Vector3& Vector3::operator+=(const Vector3& other) {
  *this = *this + other;
  return *this;
}

Vector3 Vector3::operator-(const Vector3& other) const {
  Vector3 result{};
  for (int i = 0; i < 3; ++i) {
    result[i] = data[i] - other[i];
  }
  return result;
}

Vector3& Vector3::operator-=(const Vector3& other) {
  *this = *this - other;
  return *this;
}

Vector3 Vector3::operator*(const double& other) const {
  Vector3 result{};
  for (int i = 0; i < 3; ++i) {
    result[i] = data[i] * other;
  }
  return result;
}

Vector3& Vector3::operator*=(const double& other) {
  *this = *this * other;
  return *this;
}

Vector3 Vector3::operator/(const double& other) const {
  Vector3 result{};
  for (int i = 0; i < 3; ++i) {
    result[i] = data[i] / other;
  }
  return result;
}

Vector3& Vector3::operator/=(const double& other) {
  *this = *this / other;
  return *this;
}

const Vector3 Vector3::eulerDegreesFromRotationMatrix(const Matrix3& rot_m) {
  return eulerRadiansFromRotationMatrix(rot_m) * 180.0F / M_PI;
}

const Vector3 Vector3::eulerRadiansFromRotationMatrix(const Matrix3& rot_m) {
  double x = std::atan2(rot_m[2][1], rot_m[2][2]);
  double y = -1.0 * std::atan(rot_m[2][0] / std::sqrt(1.0 - std::pow(rot_m[2][0], 2)));
  double z = std::atan2(rot_m[1][0], rot_m[0][0]);

  return Vector3({ x, y, z });
}

//===============================
// Matrix3
//===============================

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

const Matrix3 Matrix3::rotMatrixFromEulerDegrees(const Vector3& rotation_deg) {
  return rotMatrixFromEulerRadians(rotation_deg * M_PI / 180.0F);
}

const Matrix3 Matrix3::rotMatrixFromEulerRadians(const Vector3& rotation_rad) {

  Matrix3 r_x = { { { { 1, 0, 0 }, { 0, std::cos(rotation_rad.x()), -std::sin(rotation_rad.x()) }, { 0, std::sin(rotation_rad.x()), std::cos(rotation_rad.x()) } } } };
  Matrix3 r_y = { { { { std::cos(rotation_rad.y()), 0, std::sin(rotation_rad.y()) }, { 0, 1, 0 }, { -std::sin(rotation_rad.y()), 0, std::cos(rotation_rad.y()) } } } };
  Matrix3 r_z = { { { { std::cos(rotation_rad.z()), -std::sin(rotation_rad.z()), 0 }, { std::sin(rotation_rad.z()), std::cos(rotation_rad.z()), 0 }, { 0, 0, 1 } } } };

  Matrix3 r = r_z * r_y * r_x;

  return r;
}

} // namespace math

} // namespace eduart