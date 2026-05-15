#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"

namespace eduart {

namespace sensorring {

namespace device {

std::string toString(ResolutionMode mode) noexcept {
  switch (mode) {
  case ResolutionMode::RES_8X8:
    return "RES_8X8";
  case ResolutionMode::RES_8X8_LONG_RANGE:
    return "RES_8X8_LONG_RANGE";
  case ResolutionMode::RES_8X8_HIGH_ACCURACY:
    return "RES_8X8_HIGH_ACCURACY";
  case ResolutionMode::RES_16X16:
    return "RES_16X16";
  case ResolutionMode::RES_16X16_HIGH_ACCURACY:
    return "RES_16X16_HIGH_ACCURACY";
  case ResolutionMode::RES_32X32:
    return "RES_32X32";
  case ResolutionMode::RES_32X32_HIGH_ACCURACY:
    return "RES_32X32_HIGH_ACCURACY";
  case ResolutionMode::RES_48X32:
    return "RES_48X32";
  case ResolutionMode::RES_48X32_HIGH_ACCURACY:
    return "RES_48X32_HIGH_ACCURACY";
  default:
    return "UNKNOWN";
  }
}

std::ostream& operator<<(std::ostream& os, ResolutionMode mode) noexcept {
  return os << toString(mode);
}

} // namespace device

} // namespace sensorring

} // namespace eduart
