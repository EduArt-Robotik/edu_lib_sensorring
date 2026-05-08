#include "TMF8829_Measurement.hpp"

#include <sensorring_transport/ByteOperations.hpp>
#include <sensorring_transport/Protocol.hpp>

#include "sensorring/logger/Logger.hpp"

using namespace eduart::sensorring::transport;
using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_Measurement TMF8829_Measurement::fromBuffer(const std::vector<std::uint8_t>& buffer) {

  if (buffer.size() < (tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Buffer too small to contain valid TMF8829 measurement");
  }

  TMF8829_Measurement measurement;

  const std::size_t header_offset = 1 + tmf8829::RESULT_FRAME_PRE_HEADER_SIZE;         // Skip frame_id and pre-header
  const std::size_t data_offset   = header_offset + tmf8829::RESULT_FRAME_HEADER_SIZE; // Skip frame_id and pre-header and header
  const std::size_t footer_offset = buffer.size() - tmf8829::RESULT_FRAME_FOOTER_SIZE; // Footer is at the end of the buffer

  auto eof_marker = ByteOperations::readUint16(buffer, buffer.size() - 2); // Last 2 bytes of the buffer
  if (eof_marker != tmf8829::RESULT_FRAME_EOF_MARKER) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Buffer does not contain valid TMF8829 measurement: EOF marker mismatch");
  }

  measurement.header.frame_id = buffer[0];

  measurement.tmf8829_header.frame_type          = (buffer[header_offset + 0] >> 4) & 0x0F; // Upper 4 bits
  measurement.tmf8829_header.focal_plane_mode    = (buffer[header_offset + 0] >> 0) & 0x0F; // Lower 4 bits
  measurement.tmf8829_header.result_frame_format = buffer[header_offset + 1];
  measurement.tmf8829_header.payload             = ByteOperations::readUint16(buffer, header_offset + 2);
  measurement.tmf8829_header.frame_number        = ByteOperations::readUint32(buffer, header_offset + 4);
  measurement.tmf8829_header.temperature         = (static_cast<double>(buffer[header_offset + 8] + buffer[header_offset + 9] + buffer[header_offset + 10])) / 3.0; // Average of the three independent temperature sensors

  std::size_t point_buffer_size = measurement.tmf8829_header.payload - tmf8829::RESULT_FRAME_HEADER_SIZE - tmf8829::RESULT_FRAME_FOOTER_SIZE + tmf8829::RESULT_FRAME_PAYLOAD_OFFSET;
  std::size_t num_points        = point_buffer_size / 3; // Each point is 3 bytes (uint16_t distance + uint8_t snr)

  measurement.point_cloud.data.resize(num_points);
  for (std::size_t i = 0; i < num_points; i++) {
    measurement.point_cloud.data[i].raw_distance = static_cast<double>(ByteOperations::readUint16(buffer, data_offset + i * 3)) * tmf8829::DISTANCE_FIXED_POINT_FACTOR;
  }

  measurement.tmf8829_footer.timestamp_t0               = ByteOperations::readUint32(buffer, footer_offset + 0);
  measurement.tmf8829_footer.timestamp_t_last           = ByteOperations::readUint32(buffer, footer_offset + 4);
  measurement.tmf8829_footer.frame_valid                = (buffer[footer_offset + 8] >> 0) & 0x01; // Bit 0
  measurement.tmf8829_footer.warning_flag               = (buffer[footer_offset + 8] >> 3) & 0x01; // Bit 3
  measurement.tmf8829_footer.vcsel_max_power_reached    = (buffer[footer_offset + 8] >> 4) & 0x01; // Bit 4
  measurement.tmf8829_footer.vcsel_burst_limit_exceeded = (buffer[footer_offset + 8] >> 5) & 0x01; // Bit 5
  measurement.tmf8829_footer.frame_aborted              = (buffer[footer_offset + 8] >> 6) & 0x03; // Bit 6-7;

  return measurement;
}

} // namespace device

} // namespace sensorring

} // namespace eduart