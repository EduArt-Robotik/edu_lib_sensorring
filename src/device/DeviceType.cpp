#include "sensorring/device/DeviceType.hpp"

namespace eduart {

namespace sensorring {

namespace device {

std::string toString(DeviceType type) noexcept {
  switch (type) {
  case DeviceType::SensorBoard:
    return "SensorBoard";
  case DeviceType::VL53L8CX:
    return "VL53L8CX";
  case DeviceType::HTPA32:
    return "HTPA32";
  case DeviceType::WS2812b:
    return "WS2812b";
  case DeviceType::TMF8829:
    return "TMF8829";
  case DeviceType::Undefined:
    return "Undefined";
  case DeviceType::AnyDepth:
    return "AnyDepth";
  case DeviceType::AnyThermal:
    return "AnyThermal";
  case DeviceType::AnyLight:
    return "AnyLight";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const DeviceType type) noexcept {
  return os << toString(type);
}

bool isCategory(DeviceType type) noexcept {
  return type == DeviceType::AnyDepth || type == DeviceType::AnyThermal || type == DeviceType::AnyLight;
}

bool deviceMatchesExpected(DeviceType actual, DeviceType expected) noexcept {
  if (!isCategory(expected)) {
    return actual == expected;
  }
  switch (expected) {
  case DeviceType::AnyDepth:
    return actual == DeviceType::VL53L8CX || actual == DeviceType::TMF8829;
  case DeviceType::AnyThermal:
    return actual == DeviceType::HTPA32;
  case DeviceType::AnyLight:
    return actual == DeviceType::WS2812b;
  default:
    return false;
  }
}

} // namespace device

} // namespace sensorring

} // namespace eduart