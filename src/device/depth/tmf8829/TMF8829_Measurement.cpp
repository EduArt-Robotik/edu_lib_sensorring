#include "TMF8829_Measurement.hpp"

#include <sensorring_transport/ByteOperations.hpp>
#include <sensorring_transport/Protocol.hpp>

#include "sensorring/logger/Logger.hpp"

#include "TMF8829_Constants.hpp"

using namespace eduart::sensorring::transport;
using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_Measurement TMF8829_Measurement::fromBuffer(const std::vector<std::uint8_t>& buffer) {
  static constexpr double MM_TO_M = 0.001;

  // Check buffer size
  if (buffer.size() < (tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Buffer too small to contain valid TMF8829 measurement");
  }

  // Buffer offsets for different sections of the frame
  const std::size_t header_offset = 1 + tmf8829::RESULT_FRAME_PRE_HEADER_SIZE;         // Skip frame_id and pre-header
  const std::size_t data_offset   = header_offset + tmf8829::RESULT_FRAME_HEADER_SIZE; // Skip frame_id and pre-header and header
  const std::size_t footer_offset = buffer.size() - tmf8829::RESULT_FRAME_FOOTER_SIZE; // Footer is at the end of the buffer // ToDo: In a two frame transmission this reads the seconds frame footer, figure out if thats a smart thing to do

  // Check EOF marker in the footer to validate the frame
  auto eof_marker = ByteOperations::readUint16(buffer, buffer.size() - 2); // Last 2 bytes of the buffer
  if (eof_marker != tmf8829::RESULT_FRAME_EOF_MARKER) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Buffer does not contain valid TMF8829 measurement: EOF marker mismatch");
  }

  TMF8829_Measurement measurement;

  // tmf8829 header
  measurement.tmf8829_header.frame_type          = (buffer[header_offset + 0] >> 4) & 0x0F; // Upper 4 bits
  measurement.tmf8829_header.focal_plane_mode    = (buffer[header_offset + 0] >> 0) & 0x0F; // Lower 4 bits
  measurement.tmf8829_header.result_frame_format = buffer[header_offset + 1];
  measurement.tmf8829_header.payload             = ByteOperations::readUint16(buffer, header_offset + 2);
  measurement.tmf8829_header.frame_number        = ByteOperations::readUint32(buffer, header_offset + 4);
  measurement.tmf8829_header.temperature         = (static_cast<double>(buffer[header_offset + 8] + buffer[header_offset + 9] + buffer[header_offset + 10])) / 3.0; // Average of the three independent temperature sensors

  // Common DepthMeasurement fields
  measurement.header.frame_id  = buffer[0];
  measurement.header.timestamp = std::chrono::system_clock::now();
  measurement.resolution_x     = tmf8829::LOOKUP_TABLE_RESOLUTION_X[measurement.tmf8829_header.focal_plane_mode];
  measurement.resolution_y     = tmf8829::LOOKUP_TABLE_RESOLUTION_Y[measurement.tmf8829_header.focal_plane_mode];

  // tmf8829 point data
  const std::size_t point_buffer_size = measurement.tmf8829_header.payload - tmf8829::RESULT_FRAME_HEADER_SIZE - tmf8829::RESULT_FRAME_FOOTER_SIZE + tmf8829::RESULT_FRAME_PAYLOAD_OFFSET;
  const std::size_t num_points        = point_buffer_size / 3u; // ToDo: Evaluate the header layout byte for point size

  if (measurement.tmf8829_header.focal_plane_mode > 2) {
    measurement.point_cloud.data.resize(num_points * 2);

    std::size_t current_frame_data_offset      = data_offset;
    const std::size_t second_frame_data_offset = (buffer.size() - 1) / 2 + data_offset;

    for (std::size_t row = 0; row < measurement.resolution_y; row++) {

      const std::size_t result_row_idx = row * measurement.resolution_x;
      const std::size_t frame_row_idx  = (row / 2) * measurement.resolution_x;

      current_frame_data_offset = (row % 2 == 0) ? data_offset : second_frame_data_offset;

      for (std::size_t col = 0; col < measurement.resolution_x; col++) {

        const std::size_t result_point_idx = result_row_idx + col;
        const std::size_t frame_point_idx  = (frame_row_idx + col) * 3;

        measurement.point_cloud.data[result_point_idx].raw_distance = static_cast<double>(ByteOperations::readUint16(buffer, current_frame_data_offset + frame_point_idx));
        measurement.point_cloud.data[result_point_idx].raw_distance *= tmf8829::DISTANCE_FIXED_POINT_FACTOR;
        measurement.point_cloud.data[result_point_idx].raw_distance *= MM_TO_M;
        measurement.nr_valid_points++;
      }
    }

  } else {
    measurement.point_cloud.data.resize(num_points);

    for (std::size_t i = 0; i < num_points; i++) {
      measurement.point_cloud.data[i].raw_distance = static_cast<double>(ByteOperations::readUint16(buffer, data_offset + i * 3));
      measurement.point_cloud.data[i].raw_distance *= tmf8829::DISTANCE_FIXED_POINT_FACTOR;
      measurement.point_cloud.data[i].raw_distance *= MM_TO_M;
      measurement.nr_valid_points++;
    }
  }

  // tmf8829 footer
  measurement.tmf8829_footer.timestamp_t0               = ByteOperations::readUint32(buffer, footer_offset + 0);
  measurement.tmf8829_footer.timestamp_t_last           = ByteOperations::readUint32(buffer, footer_offset + 4);
  measurement.tmf8829_footer.frame_valid                = (buffer[footer_offset + 8] >> 0) & 0x01; // Bit 0
  measurement.tmf8829_footer.warning_flag               = (buffer[footer_offset + 8] >> 3) & 0x01; // Bit 3
  measurement.tmf8829_footer.vcsel_max_power_reached    = (buffer[footer_offset + 8] >> 4) & 0x01; // Bit 4
  measurement.tmf8829_footer.vcsel_burst_limit_exceeded = (buffer[footer_offset + 8] >> 5) & 0x01; // Bit 5
  measurement.tmf8829_footer.frame_aborted              = (buffer[footer_offset + 8] >> 6) & 0x03; // Bit 6-7;

  // Common DepthMeasurement fields
  measurement.header.state = measurement.tmf8829_footer.frame_valid ? device::DeviceState::Ok : device::DeviceState::Error;

  return measurement;
}

} // namespace device

} // namespace sensorring

} // namespace eduart