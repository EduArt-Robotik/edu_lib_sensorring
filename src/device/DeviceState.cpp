#include "sensorring/device/DeviceState.hpp"

namespace eduart {

namespace sensorring {

namespace device {

std::string toString(DeviceState state) noexcept {
  switch (state) {
  case DeviceState::Undefined:
    return "Undefined";
  case DeviceState::Initialized:
    return "Initialized";
  case DeviceState::Idle:
    return "Idle";
  case DeviceState::Ok:
    return "Ok";
  case DeviceState::ReceiveError:
    return "ReceiveError";
  case DeviceState::ProcessError:
    return "ProcessError";
  case DeviceState::Error:
    return "Error";
  case DeviceState::Shutdown:
    return "Shutdown";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, DeviceState state) noexcept {
  return os << toString(state);
}

} // namespace device

} // namespace sensorring

} // namespace eduart
