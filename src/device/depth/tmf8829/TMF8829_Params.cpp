#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"

#include "TMF8829_Constants.hpp"

namespace eduart {

namespace sensorring {

namespace device {

unsigned int getXResolution(ResolutionMode mode) {
  return tmf8829::LOOKUP_TABLE_RESOLUTION_X_RES_MODE[static_cast<std::size_t>(mode)];
}

unsigned int getYResolution(ResolutionMode mode) {
  return tmf8829::LOOKUP_TABLE_RESOLUTION_Y_RES_MODE[static_cast<std::size_t>(mode)];
}

std::string toString(ResolutionMode mode) noexcept {
  switch (mode) {
  case ResolutionMode::Res8x8:
    return "Res8x8";
  case ResolutionMode::Res8x8LongRange:
    return "Res8x8LongRange";
  case ResolutionMode::Res8x8HighAccuracy:
    return "Res8x8HighAccuracy";
  case ResolutionMode::Res16x16:
    return "Res16x16";
  case ResolutionMode::Res16x16HighAccuracy:
    return "Res16x16HighAccuracy";
  case ResolutionMode::Res32x32:
    return "Res32x32";
  case ResolutionMode::Res32x32HighAccuracy:
    return "Res32x32HighAccuracy";
  case ResolutionMode::Res48x32:
    return "Res48x32";
  case ResolutionMode::Res48x32HighAccuracy:
    return "Res48x32HighAccuracy";
  default:
    return "UNKNOWN";
  }
}

std::ostream& operator<<(std::ostream& os, ResolutionMode mode) noexcept {
  return os << toString(mode);
}

std::size_t TMF8829_Params::calculateResultFrameSize() const {
  const auto point_size   = result_format.calculatePointSize();
  const auto nr_of_points = tmf8829::LOOKUP_TABLE_RESOLUTION_X_FP_MODE[static_cast<std::size_t>(resolution_mode)] * tmf8829::LOOKUP_TABLE_RESOLUTION_Y_FP_MODE[static_cast<std::size_t>(resolution_mode)];

  // High resolution modes send their points in two equal sized frames
  const auto points_per_frame = (resolution_mode > ResolutionMode::Res16x16HighAccuracy) ? nr_of_points / 2 : nr_of_points;
  const auto total_size       = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + point_size * points_per_frame;

  return total_size;
}

bool TMF8829_Params::isResultSizeValid() const {
  return calculateResultFrameSize() <= tmf8829::MAX_RESULT_FRAME_SIZE;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
