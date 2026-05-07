#include "sensorring/board/SensorBoardType.hpp"

namespace eduart {

namespace sensorring {

namespace device {

std::string toString(SensorBoardType type) noexcept {
  switch (type) {
  case SensorBoardType::Sidepanel:
    return "Sidepanel";
  case SensorBoardType::Headlight:
    return "Headlight";
  case SensorBoardType::Taillight:
    return "Taillight";
  case SensorBoardType::Minipanel:
    return "Minipanel";
  case SensorBoardType::Undefined:
    return "Undefined";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const SensorBoardType type) noexcept {
  return os << toString(type);
}

} // namespace device

} // namespace sensorring

} // namespace eduart