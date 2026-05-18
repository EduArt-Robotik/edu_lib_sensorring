#pragma once

#include <cstdint>
#include <vector>

#include "sensorring/device/depth/tmf8829/TMF8829_ResultFormat.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"

namespace eduart {

namespace sensorring {

namespace device {

struct TMF8829_Measurement : public measurement::DepthMeasurement {

  struct TMF8829_Header {
    std::uint8_t frame_type;
    std::uint8_t focal_plane_mode;
    std::uint8_t result_frame_format;
    std::uint16_t payload;
    std::uint16_t frame_number;
    double temperature;
  };

  struct TMF8829_Footer {
    std::uint32_t timestamp_t0;
    std::uint32_t timestamp_t_last;
    std::uint8_t frame_valid;
    std::uint8_t warning_flag;
    std::uint8_t vcsel_max_power_reached;
    std::uint8_t vcsel_burst_limit_exceeded;
    std::uint8_t frame_aborted;
  };

  TMF8829_Header tmf8829_header;
  TMF8829_Footer tmf8829_footer;
  TMF8829_ResultFormat tmf8829_result_format;

  /**
   * @brief Parse a TMF8829 frame from a raw byte buffer.
   * @param[in] buffer Raw bytes received from the device. Buffer has to be at least large enough to contain a header and footer, otherwise parsing will fail.
   * @return Parsed TMF8829_Measurement struct.
   * @throws std::runtime_error if the buffer is too small to contain a valid frame.
   */
  static TMF8829_Measurement fromBuffer(const std::vector<std::uint8_t>& buffer);
};

} // namespace device

} // namespace sensorring

} // namespace eduart