#include "sensorring/board/SensorBoardParams.hpp"

namespace eduart {

namespace sensorring {

namespace board {

std::string toString(Orientation orientation) noexcept {
  switch (orientation) {
  case Orientation::Left:
    return "Left";
  case Orientation::Right:
    return "Right";
  case Orientation::None:
    return "None";
  default:
    return "UNKNOWN";
  }
}

std::ostream& operator<<(std::ostream& os, Orientation orientation) noexcept {
  return os << toString(orientation);
}

} // namespace board

} // namespace sensorring

} // namespace eduart
