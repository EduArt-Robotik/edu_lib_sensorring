#include "sensorring/math/Pose.hpp"

namespace eduart {

namespace sensorring {

namespace math {

Pose Pose::operator+(const Pose& other) const {
  Pose result;
  result.translation = this->translation + other.translation;
  result.orientation = this->orientation + other.orientation;
  return result;
}

Pose Pose::operator-(const Pose& other) const {
  Pose result;
  result.translation = this->translation - other.translation;
  result.orientation = this->orientation - other.orientation;
  return result;
}

} // namespace math

} // namespace sensorring

} // namespace eduart
