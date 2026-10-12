#include "sensorring/interface/ComInterfaceID.hpp"

namespace eduart {

namespace sensorring {

namespace com {

std::string toString(InterfaceType type) noexcept {
  switch (type) {
  case InterfaceType::Undefined:
    return "Undefined";
  case InterfaceType::SocketCan:
    return "SocketCAN";
  case InterfaceType::UsbTingo:
    return "USBtingo";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const InterfaceType type) noexcept {
  return os << toString(type);
}

} // namespace com

} // namespace sensorring

} // namespace eduart
