#include "sensorring/interface/ComEndpoint.hpp"

namespace eduart {

namespace sensorring {

namespace com {

namespace {
const char* deviceIdToName(std::uint8_t id) {
  switch (id) {
  case 0x00:
    return "Board";
  case 0x01:
    return "VL53L8CX";
  case 0x02:
    return "HTPA32";
  case 0x03:
    return "WS2812b";
  default:
    return nullptr;
  }
}
} // namespace

bool ComEndpoint::operator==(const ComEndpoint& other) const {
  return direction == other.direction && boardAddress == other.boardAddress && deviceId == other.deviceId;
}

bool ComEndpoint::operator!=(const ComEndpoint& other) const {
  return !(*this == other);
}

std::string ComEndpoint::toString() const {
  std::string result = (direction == Direction::Input) ? "Input/" : "Output/";

  if (boardAddress == BROADCAST) {
    result += "Broadcast/";
  } else if (boardAddress == ANY_BOARD) {
    result += "ANY/";
  } else {
    result += "Board" + std::to_string(boardAddress - 1) + "/";
  }

  const char* name = deviceIdToName(deviceId);
  if (name) {
    result += name;
  } else {
    result += "Dev" + std::to_string(deviceId);
  }
  return result;
}

} // namespace com

} // namespace sensorring

} // namespace eduart
