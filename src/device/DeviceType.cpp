#include "sensorring/device/DeviceType.hpp"

namespace eduart {

namespace sensorring {

namespace device {

std::string toString(DeviceType type) noexcept {
  switch (type) {
  case DeviceType::VL53L8CX:
    return "VL53L8CX";
  case DeviceType::HTPA32:
    return "HTPA32";
  case DeviceType::WS2812b:
    return "WS2812b";
  case DeviceType::TMF8829:
    return "TMF8829";
  case DeviceType::UNDEFINED:
    return "Undefined";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const DeviceType type) noexcept {
  return os << toString(type);
}

} // namespace device

} // namespace sensorring

} // namespace eduart