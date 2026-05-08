#pragma once

#include <cstdint>

namespace eduart {

namespace sensorring {

namespace device {

namespace tmf8829 {

// TMF8829 result frame element sizes
constexpr std::uint8_t RESULT_FRAME_PRE_HEADER_SIZE = 5u;
constexpr std::uint8_t RESULT_FRAME_HEADER_SIZE     = 16u;
constexpr std::uint8_t RESULT_FRAME_FOOTER_SIZE     = 12u;

// TMF8829 result frame constants
constexpr std::uint16_t RESULT_FRAME_EOF_MARKER    = 0xE0F7; // End-of-frame marker
constexpr std::uint8_t RESULT_FRAME_PAYLOAD_OFFSET = 4u;     // Offset to add to the payload field of the header to get the actual length including header, point data and footer
constexpr double DISTANCE_FIXED_POINT_FACTOR       = 0.25l;  // Each unit in the raw distance corresponds to 0.25 mm

constexpr std::uint8_t LOOKUP_TABLE_RESOLUTION_X[] = { 8, 8, 16, 32, 32, 32, 48 };
constexpr std::uint8_t LOOKUP_TABLE_RESOLUTION_Y[] = { 8, 8, 16, 32, 32, 32, 32 };

} // namespace tmf8829

} // namespace device

} // namespace sensorring

} // namespace eduart