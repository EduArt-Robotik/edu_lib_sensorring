#include "TMF8829_Measurement.hpp"

#include <sensorring_transport/ByteOperations.hpp>
#include <sensorring_transport/Protocol.hpp>

#include "sensorring/device/depth/tmf8829/TMF8829_ResultFormat.hpp"
#include "sensorring/logger/Logger.hpp"

#include "TMF8829_Constants.hpp"

using namespace eduart::sensorring::transport;
using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @brief Calculate x, y, z coordinates from tmf8829 point data according to the tmf8829 python driver implementation
 * @param[in] x Column index of the point in the tmf8829 frame
 * @param[in] y Row index of the point in the tmf8829 frame
 * @param[in] x_res Total number of columns in the tmf8829 frame
 * @param[in] y_res Total number of rows in the tmf8829 frame
 * @param[in,out] distance Raw distance value of the point in mm, will be modified by the correction factor
 * @return Vector3 containing the x, y, z coordinates of the point
 */
math::Vector3 calculateXYZ(unsigned int x, unsigned int y, unsigned int x_res, unsigned int y_res, double& distance) {

  // Calculate correction factors according to the tmf8829 python driver
  // ToDo: Replace by LUT, update LUT on every resolution change
  const double x_corr = (x - ((x_res / 2.0) + 0.5)) / (x_res * 3.0 / 2.0);
  const double y_corr = (y - ((y_res / 2.0) + 0.5)) / y_res;
  const double z_corr = std::sqrt(1 + x_corr * x_corr + y_corr * y_corr);

  distance /= z_corr;

  math::Vector3 point;
  point.x() = distance * x_corr;
  point.y() = distance * y_corr;
  point.z() = distance;

  return point;
}

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

  // result format
  const auto& fmt                                   = measurement.tmf8829_header.result_frame_format;
  measurement.tmf8829_result_format.full_noise      = (fmt & tmf8829::RESULT_FRAME_FULL_NOISE_MASK) != 0;
  measurement.tmf8829_result_format.xtalk           = (fmt & tmf8829::RESULT_FRAME_XTALK_MASK) != 0;
  measurement.tmf8829_result_format.noise_strength  = (fmt & tmf8829::RESULT_FRAME_NOISE_STRENGTH_MASK) != 0;
  measurement.tmf8829_result_format.signal_strength = (fmt & tmf8829::RESULT_FRAME_SIGNAL_STRENGTH_MASK) != 0;
  measurement.tmf8829_result_format.nr_of_peaks     = (fmt & tmf8829::RESULT_FRAME_SIGNAL_NR_PEAKS_MASK);
  const std::size_t point_size                      = measurement.tmf8829_result_format.calculatePointSize();

  // Common DepthMeasurement fields
  measurement.header.frame_id  = buffer[0];
  measurement.header.timestamp = std::chrono::system_clock::now();
  measurement.resolution_x     = tmf8829::LOOKUP_TABLE_RESOLUTION_X_FP_MODE[measurement.tmf8829_header.focal_plane_mode];
  measurement.resolution_y     = tmf8829::LOOKUP_TABLE_RESOLUTION_Y_FP_MODE[measurement.tmf8829_header.focal_plane_mode];

  // tmf8829 point data
  const std::size_t point_buffer_size = measurement.tmf8829_header.payload - tmf8829::RESULT_FRAME_HEADER_SIZE - tmf8829::RESULT_FRAME_FOOTER_SIZE + tmf8829::RESULT_FRAME_PAYLOAD_OFFSET;
  const std::size_t num_points        = point_buffer_size / point_size;

  const bool is_double_frame_mode            = measurement.tmf8829_header.focal_plane_mode > 2;
  const std::size_t total_num_points         = is_double_frame_mode ? num_points * 2u : num_points;
  const std::size_t second_frame_data_offset = (buffer.size() - 1) / 2 + data_offset;

  measurement.point_cloud.data.resize(total_num_points);

  for (std::size_t row = 0; row < measurement.resolution_y; row++) {

    const std::size_t result_row_idx            = row * measurement.resolution_x;
    const std::size_t frame_row_idx             = (is_double_frame_mode ? (row / 2) : row) * measurement.resolution_x;
    const std::size_t current_frame_data_offset = (is_double_frame_mode && (row % 2u == 1u)) ? second_frame_data_offset : data_offset;

    for (std::size_t col = 0; col < measurement.resolution_x; col++) {

      const std::size_t result_point_idx = result_row_idx + col;
      const std::size_t frame_point_idx  = (frame_row_idx + col) * point_size;

      std::size_t read_idx = frame_point_idx;

      // 2 bytes noise
      if (measurement.tmf8829_result_format.noise_strength) {
        // Read noise strength
        read_idx += 2;
      }

      // 2 bytes xtalk
      if (measurement.tmf8829_result_format.xtalk) {
        // Read xtalk
        read_idx += 2;
      }

      for (std::size_t peak_idx = 0; peak_idx < measurement.tmf8829_result_format.nr_of_peaks; peak_idx++) {
        // Peak n: 2 bytes distance
        measurement.point_cloud.data[result_point_idx].raw_distance = static_cast<double>(ByteOperations::readUint16(buffer, current_frame_data_offset + read_idx));
        measurement.point_cloud.data[result_point_idx].raw_distance *= tmf8829::DISTANCE_FIXED_POINT_FACTOR;
        measurement.point_cloud.data[result_point_idx].raw_distance *= MM_TO_M;
        read_idx += 2;

        // Peak n: 1 byte Snr
        if (measurement.tmf8829_result_format.signal_strength) {
          // Read noise strength
          read_idx += 1;
        }

        // Peak n: 2 bytes signal strength
        if (measurement.tmf8829_result_format.signal_strength) {
          // Read signal strength
          read_idx += 2;
        }

        // Populate x, y, z coordinated according to the tmf8829 python driver implementation
        //calculateXYZ(col, result_row_idx, measurement.resolution_x, measurement.resolution_y, measurement.point_cloud.data[result_point_idx].raw_distance);
        measurement.point_cloud.data[result_point_idx].point = calculateXYZ(col, result_row_idx, measurement.resolution_x, measurement.resolution_y, measurement.point_cloud.data[result_point_idx].raw_distance);
      }

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