#include <catch2/catch_all.hpp>

#include <cstdint>
#include <vector>

#include "device/depth/tmf8829/TMF8829_Constants.hpp"
#include "device/depth/tmf8829/TMF8829_Measurement.hpp"

using eduart::sensorring::device::TMF8829_Measurement;
namespace tmf8829 = eduart::sensorring::device::tmf8829;

namespace {

constexpr std::size_t PRE_HEADER_OFFSET = 1u;
constexpr std::size_t HEADER_OFFSET     = PRE_HEADER_OFFSET + tmf8829::RESULT_FRAME_PRE_HEADER_SIZE;
constexpr std::size_t DATA_OFFSET       = HEADER_OFFSET + tmf8829::RESULT_FRAME_HEADER_SIZE;

void writeUint16LE(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint16_t value) {
  buffer[offset + 0] = static_cast<std::uint8_t>(value & 0xFFu);
  buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
}

void writeUint32LE(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint32_t value) {
  buffer[offset + 0] = static_cast<std::uint8_t>(value & 0xFFu);
  buffer[offset + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
  buffer[offset + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFFu);
  buffer[offset + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFFu);
}

void setupCommonHeaderAndFooter(std::vector<std::uint8_t>& buffer, std::uint8_t focal_plane_mode, std::uint16_t payload) {
  buffer[0] = 0x2Au;

  buffer[HEADER_OFFSET + 0] = static_cast<std::uint8_t>((0x1u << 4) | (focal_plane_mode & 0x0Fu));
  buffer[HEADER_OFFSET + 1] = 0x00u;
  writeUint16LE(buffer, HEADER_OFFSET + 2, payload);
  writeUint32LE(buffer, HEADER_OFFSET + 4, 0x12345678u);

  buffer[HEADER_OFFSET + 8]  = 30u;
  buffer[HEADER_OFFSET + 9]  = 31u;
  buffer[HEADER_OFFSET + 10] = 32u;

  const std::size_t footer_offset = buffer.size() - tmf8829::RESULT_FRAME_FOOTER_SIZE;
  writeUint32LE(buffer, footer_offset + 0, 111u);
  writeUint32LE(buffer, footer_offset + 4, 222u);
  buffer[footer_offset + 8] = 0x01u; // frame_valid

  writeUint16LE(buffer, buffer.size() - 2u, tmf8829::RESULT_FRAME_EOF_MARKER);
}

double toMeters(std::uint16_t raw_units) {
  return static_cast<double>(raw_units) * tmf8829::DISTANCE_FIXED_POINT_FACTOR * 0.001;
}

} // namespace

TEST_CASE("TMF8829 fromBuffer decodes single-frame payload layout", "[TMF8829][DepthMeasurement]") {
  constexpr std::uint8_t focal_plane_mode = 0u; // 8x8
  constexpr std::size_t resolution_x      = 8u;
  constexpr std::size_t resolution_y      = 8u;
  constexpr std::size_t num_points        = resolution_x * resolution_y;
  constexpr std::uint16_t payload         = static_cast<std::uint16_t>(num_points * 3u + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE - tmf8829::RESULT_FRAME_PAYLOAD_OFFSET);

  std::vector<std::uint8_t> buffer(DATA_OFFSET + num_points * 3u + tmf8829::RESULT_FRAME_FOOTER_SIZE, 0u);
  setupCommonHeaderAndFooter(buffer, focal_plane_mode, payload);

  for (std::size_t i = 0; i < num_points; i++) {
    const std::uint16_t raw = static_cast<std::uint16_t>(i + 1u);
    writeUint16LE(buffer, DATA_OFFSET + i * 3u, raw);
  }

  const auto measurement = TMF8829_Measurement::fromBuffer(buffer);

  REQUIRE(measurement.point_cloud.data.size() == num_points);
  REQUIRE(measurement.nr_valid_points == num_points);
  REQUIRE(measurement.point_cloud.data[0].raw_distance == Catch::Approx(toMeters(1u)));
  REQUIRE(measurement.point_cloud.data[17].raw_distance == Catch::Approx(toMeters(18u)));
  REQUIRE(measurement.point_cloud.data[num_points - 1u].raw_distance == Catch::Approx(toMeters(static_cast<std::uint16_t>(num_points))));
}

TEST_CASE("TMF8829 fromBuffer decodes double-frame row interleaving", "[TMF8829][DepthMeasurement]") {
  constexpr std::uint8_t focal_plane_mode = 3u; // 32x32
  constexpr std::size_t resolution_x      = 32u;
  constexpr std::size_t resolution_y      = 32u;
  constexpr std::size_t half_points       = (resolution_x * resolution_y) / 2u;
  constexpr std::uint16_t payload         = static_cast<std::uint16_t>(half_points * 3u + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE - tmf8829::RESULT_FRAME_PAYLOAD_OFFSET);

  // Keep size odd so (size-1)/2 is exact and place footer after both frame payloads.
  std::vector<std::uint8_t> buffer(3139u, 0u);
  setupCommonHeaderAndFooter(buffer, focal_plane_mode, payload);

  const std::size_t second_frame_data_offset = (buffer.size() - 1u) / 2u + DATA_OFFSET;

  for (std::size_t i = 0; i < half_points; i++) {
    writeUint16LE(buffer, DATA_OFFSET + i * 3u, static_cast<std::uint16_t>(1000u + i));
    writeUint16LE(buffer, second_frame_data_offset + i * 3u, static_cast<std::uint16_t>(2000u + i));
  }

  const auto measurement = TMF8829_Measurement::fromBuffer(buffer);

  REQUIRE(measurement.point_cloud.data.size() == resolution_x * resolution_y);
  REQUIRE(measurement.nr_valid_points == resolution_x * resolution_y);

  const std::size_t idx_row0_col0  = 0u;
  const std::size_t idx_row1_col0  = resolution_x;
  const std::size_t idx_row2_col5  = 2u * resolution_x + 5u;
  const std::size_t idx_row3_col5  = 3u * resolution_x + 5u;
  const std::size_t idx_row31_col31 = 31u * resolution_x + 31u;

  REQUIRE(measurement.point_cloud.data[idx_row0_col0].raw_distance == Catch::Approx(toMeters(1000u)));
  REQUIRE(measurement.point_cloud.data[idx_row1_col0].raw_distance == Catch::Approx(toMeters(2000u)));
  REQUIRE(measurement.point_cloud.data[idx_row2_col5].raw_distance == Catch::Approx(toMeters(1000u + 32u + 5u)));
  REQUIRE(measurement.point_cloud.data[idx_row3_col5].raw_distance == Catch::Approx(toMeters(2000u + 32u + 5u)));
  REQUIRE(measurement.point_cloud.data[idx_row31_col31].raw_distance == Catch::Approx(toMeters(2000u + 15u * 32u + 31u)));
}
