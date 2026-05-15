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
constexpr std::uint16_t RESULT_FRAME_EOF_MARKER          = 0xE0F7; // End-of-frame marker
constexpr std::uint16_t RESULT_FRAME_MAX_PAYLOAD_SIZE    = 8192u;  // Maximum allowed payload size of a single TMF8829 result frame, including header, point data and footer
constexpr std::uint8_t RESULT_FRAME_PAYLOAD_OFFSET       = 4u;     // Offset to add to the payload field of the header to get the actual length including header, point data and footer
constexpr std::uint8_t RESULT_FRAME_FULL_NOISE_MASK      = 0x80;   // Offset to add to the payload field of the header to get the actual length including header, point data and footer
constexpr std::uint8_t RESULT_FRAME_XTALK_MASK           = 0x20;   // Offset to add to the payload field of the header to get the actual length including header, point data and footer
constexpr std::uint8_t RESULT_FRAME_NOISE_STRENGTH_MASK  = 0x10;   // Offset to add to the payload field of the header to get the actual length including header, point data and footer
constexpr std::uint8_t RESULT_FRAME_SIGNAL_STRENGTH_MASK = 0x08;   // Offset to add to the payload field of the header to get the actual length including header, point data and footer
constexpr std::uint8_t RESULT_FRAME_SIGNAL_NR_PEAKS_MASK = 0x07;   // Offset to add to the payload field of the header to get the actual length including header, point data and footer

// TMFF8829 point data constants
constexpr double DISTANCE_FIXED_POINT_FACTOR = 0.25l; // Each unit in the raw distance corresponds to 0.25 mm

constexpr std::uint8_t LOOKUP_TABLE_RESOLUTION_X[] = { 8, 8, 16, 32, 32, 48 };
constexpr std::uint8_t LOOKUP_TABLE_RESOLUTION_Y[] = { 8, 8, 16, 32, 32, 32 };

} // namespace tmf8829

} // namespace device

} // namespace sensorring

} // namespace eduart