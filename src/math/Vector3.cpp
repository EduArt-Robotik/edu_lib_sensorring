#include <cmath>

#include "sensorring/math/Math.hpp"
#include "sensorring/math/Matrix3.hpp"

namespace eduart {

namespace math {

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

double Vector3::abs() const {
  return std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
}

} // namespace math

} // namespace eduart