#include "sensorring/MeasurementClient.hpp"

namespace eduart {

namespace manager {

std::string to_string(ManagerState state) {
  switch (state) {
  case ManagerState::Uninitialized:
    return "Uninitialized";
  case ManagerState::Initialized:
    return "Initialized";
  case ManagerState::Running:
    return "Running";
  case ManagerState::Shutdown:
    return "Shutdown";
  case ManagerState::Error:
    return "Error";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, ManagerState state) {
  return os << to_string(state);
}

} // namespace manager

} // namespace eduart