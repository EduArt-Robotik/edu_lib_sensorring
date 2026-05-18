#include <catch2/catch_all.hpp>

#include "device/depth/tmf8829/TMF8829_Constants.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"

using eduart::sensorring::device::ResolutionMode;
using eduart::sensorring::device::TMF8829_Params;
namespace tmf8829 = eduart::sensorring::device::tmf8829;

// RES_16X16 gives 32x32 = 1024 points per frame (mode <= RES_16X16_HIGH_ACCURACY, not halved).
// Frame overhead = PRE_HEADER(5) + HEADER(16) + FOOTER(12) = 33 bytes.
// Limit = MAX_RESULT_FRAME_SIZE = 8192 bytes.
//
// point_size = 7  ->  total = 33 + 1024 * 7 = 7201  (valid,   <= 8192)
// point_size = 8  ->  total = 33 + 1024 * 8 = 8225  (invalid, >  8192)
//
// These are the tightest integer point-size brackets around the 8192-byte boundary
// for the 1024-point resolution mode.

TEST_CASE("TMF8829_Params isResultSizeValid edge cases around 8192-byte limit", "[TMF8829][Params]") {
  TMF8829_Params params;
  params.resolution_mode = ResolutionMode::RES_16X16; // 1024 points per frame

  SECTION("returns true when total frame size is just below limit (point_size=7, total=7201)") {
    // calculatePointSize: base=3, +signal_strength=5, *nr_of_peaks(1)=5, +xtalk=7  =>  7
    params.result_format.signal_strength = true;
    params.result_format.nr_of_peaks     = 1;
    params.result_format.xtalk           = true;
    params.result_format.noise_strength  = false;

    constexpr std::size_t expected_total = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + 1024u * 7u;
    STATIC_REQUIRE(expected_total == 7201u);
    STATIC_REQUIRE(expected_total <= tmf8829::MAX_RESULT_FRAME_SIZE);

    REQUIRE(params.isResultSizeValid() == true);
  }

  SECTION("returns false when total frame size just exceeds limit (point_size=8, total=8225)") {
    // calculatePointSize: base=3, *nr_of_peaks(2)=6, +xtalk=8  =>  8
    params.result_format.signal_strength = false;
    params.result_format.nr_of_peaks     = 2;
    params.result_format.xtalk           = true;
    params.result_format.noise_strength  = false;

    constexpr std::size_t expected_total = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + 1024u * 8u;
    STATIC_REQUIRE(expected_total == 8225u);
    STATIC_REQUIRE(expected_total > tmf8829::MAX_RESULT_FRAME_SIZE);

    REQUIRE(params.isResultSizeValid() == false);
  }
}

// RES_32X32 (mode 5) is > RES_16X16_HIGH_ACCURACY (4), so measurements are split across two equal sized frames.
// Points per resolution: LOOKUP_X[5]=48, LOOKUP_Y[5]=32 → 1536 total → 768 per frame.
// Frame overhead = PRE_HEADER(5) + HEADER(16) + FOOTER(12) = 33 bytes.
//
// point_size=6  ->  half_total = 33 + 768 * 6  = 4641  (valid,   <= 8192; full=9249 would exceed limit)
// point_size=11 ->  half_total = 33 + 768 * 11 = 8481  (invalid, >  8192 even per-half-frame)

TEST_CASE("TMF8829_Params isResultSizeValid double-frame split for high-res modes", "[TMF8829][Params]") {
  TMF8829_Params params;
  params.resolution_mode = ResolutionMode::RES_32X32; // mode > RES_16X16_HIGH_ACCURACY → halved points per frame

  SECTION("returns true when each half frame fits within limit (point_size=6, half_total=4641)") {
    // calculatePointSize: base=3, *nr_of_peaks(2)=6  =>  6
    params.result_format.signal_strength = false;
    params.result_format.nr_of_peaks     = 2;
    params.result_format.xtalk           = false;
    params.result_format.noise_strength  = false;

    constexpr std::size_t nr_of_points     = 48u * 32u;         // 1536 — LOOKUP_X[5] * LOOKUP_Y[5]
    constexpr std::size_t points_per_frame = nr_of_points / 2u; // 768  — halved due to split
    constexpr std::size_t half_frame_total = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + points_per_frame * 6u;
    constexpr std::size_t full_frame_total = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + nr_of_points * 6u;

    STATIC_REQUIRE(half_frame_total == 4641u);
    STATIC_REQUIRE(half_frame_total <= tmf8829::MAX_RESULT_FRAME_SIZE);
    STATIC_REQUIRE(full_frame_total == 9249u);
    STATIC_REQUIRE(full_frame_total > tmf8829::MAX_RESULT_FRAME_SIZE); // would fail without the split

    REQUIRE(params.isResultSizeValid() == true);
  }

  SECTION("returns false when each half frame still exceeds limit (point_size=11, half_total=8481)") {
    // calculatePointSize: base=3, *nr_of_peaks(3)=9, +xtalk=11  =>  11
    params.result_format.signal_strength = false;
    params.result_format.nr_of_peaks     = 3;
    params.result_format.xtalk           = true;
    params.result_format.noise_strength  = false;

    constexpr std::size_t points_per_frame = 48u * 32u / 2u; // 768
    constexpr std::size_t half_frame_total = tmf8829::RESULT_FRAME_PRE_HEADER_SIZE + tmf8829::RESULT_FRAME_HEADER_SIZE + tmf8829::RESULT_FRAME_FOOTER_SIZE + points_per_frame * 11u;

    STATIC_REQUIRE(half_frame_total == 8481u);
    STATIC_REQUIRE(half_frame_total > tmf8829::MAX_RESULT_FRAME_SIZE);

    REQUIRE(params.isResultSizeValid() == false);
  }
}
