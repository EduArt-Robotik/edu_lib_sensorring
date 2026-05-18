#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"

#include "TMF8829_Constants.hpp"

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

std::size_t TMF8829_Params::calculateResultFrameSize() const {
  const auto point_size   = result_format.calculatePointSize();
  const auto nr_of_points = tmf8829::LOOKUP_TABLE_RESOLUTION_X[static_cast<std::size_t>(resolution_mode)] * tmf8829::LOOKUP_TABLE_RESOLUTION_Y[static_cast<std::size_t>(resolution_mode)];

  // High resolution modes send their points in two equal sized frames
  const auto points_per_frame = (resolution_mode > ResolutionMode::RES_16X16_HIGH_ACCURACY) ? nr_of_points / 2 : nr_of_points;
  const auto total_size       = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + point_size * points_per_frame;

  return total_size;
}

bool TMF8829_Params::isResultSizeValid() const {
  return calculateResultFrameSize() <= tmf8829::MAX_RESULT_FRAME_SIZE;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
