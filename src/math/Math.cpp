#include "sensorring/math/Math.hpp"

namespace eduart {

namespace math {

const Vector3 eulerDegreesFromRotationMatrix(const Matrix3& rot_m) {
  return eulerRadiansFromRotationMatrix(rot_m) * 180.0F / M_PI;
}

const Vector3 eulerRadiansFromRotationMatrix(const Matrix3& rot_m) {
  double x = std::atan2(rot_m[2][1], rot_m[2][2]);
  double y = -1.0 * std::atan(rot_m[2][0] / std::sqrt(1.0 - std::pow(rot_m[2][0], 2)));
  double z = std::atan2(rot_m[1][0], rot_m[0][0]);

  return Vector3({ x, y, z });
}

const Matrix3 rotMatrixFromEulerDegrees(const Vector3& rotation_deg) {
  return rotMatrixFromEulerRadians(rotation_deg * M_PI / 180.0F);
}

const Matrix3 rotMatrixFromEulerRadians(const Vector3& rotation_rad) {

  Matrix3 r_x = { { { { 1, 0, 0 }, { 0, std::cos(rotation_rad.x()), -std::sin(rotation_rad.x()) }, { 0, std::sin(rotation_rad.x()), std::cos(rotation_rad.x()) } } } };
  Matrix3 r_y = { { { { std::cos(rotation_rad.y()), 0, std::sin(rotation_rad.y()) }, { 0, 1, 0 }, { -std::sin(rotation_rad.y()), 0, std::cos(rotation_rad.y()) } } } };
  Matrix3 r_z = { { { { std::cos(rotation_rad.z()), -std::sin(rotation_rad.z()), 0 }, { std::sin(rotation_rad.z()), std::cos(rotation_rad.z()), 0 }, { 0, 0, 1 } } } };

  Matrix3 r = r_z * r_y * r_x;

  return r;
}

} // namespace math

} // namespace eduart