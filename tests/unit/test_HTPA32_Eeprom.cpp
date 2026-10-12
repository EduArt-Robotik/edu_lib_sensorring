// Unit tests for eduart::sensorring::device::htpa32::HTPA32_Eeprom serialize/deserialize roundtrip.

#include <algorithm>
#include <array>
#include <catch2/catch_all.hpp>

#include "device/thermal/htpa32/HTPA32_Eeprom.hpp"

using eduart::sensorring::device::htpa32::HTPA32_Eeprom;

namespace {

void fill_test_eeprom(HTPA32_Eeprom& eeprom) {
  auto& d = eeprom.data;

  d.pixc_min = -1.25f;
  d.pixc_max = 42.5f;

  d.grad_scale  = 7;
  d.tablenumber = 1234u;

  d.epsilon    = 95;
  d.mbit_calib = 1;
  d.bias_calib = 2;
  d.clk_calib  = 3;
  d.bpa_calib  = 4;
  d.pu_calib   = 5;
  d.arraytype  = 6;

  d.vddth1 = 0x1234;
  d.vddth2 = 0xABCD;

  d.ptat_gradient = 0.125f;
  d.ptat_offset   = -0.5f;

  d.ptat_th1 = 0x0102;
  d.ptat_th2 = 0x0304;

  d.vddsc_gradient = 9;
  d.vddsc_offset   = 10;
  d.global_offset  = 11;

  d.global_gain = 0x0F0F;

  d.mbit_user = 12;
  d.bias_user = 13;
  d.clk_user  = 14;
  d.bpa_user  = 15;
  d.pu_user   = 16;

  d.device_id     = 0xDEADBEEF;
  d.norof_deadpix = 3;

  for (int i = 0; i < 24; ++i) {
    d.deadpix_addr[i] = static_cast<uint16_t>(0x1000u + static_cast<uint16_t>(i));
  }
  for (int i = 0; i < 12; ++i) {
    d.deadpix_mask[i] = static_cast<uint16_t>(0x0101u * static_cast<uint16_t>(i + 1));
  }

  for (int i = 0; i < 256; ++i) {
    d.vddcomp_gradient[i] = static_cast<int16_t>(-128 + i);
    d.vddcomp_offset[i]   = static_cast<int16_t>(i * 2 - 256);
  }

  for (int i = 0; i < 1024; ++i) {
    d.th_gradient[i] = static_cast<int16_t>(i - 512);
    d.th_offset[i]   = static_cast<int16_t>(512 - i);
  }

  for (int i = 0; i < 1024; ++i) {
    d.p[i] = static_cast<uint16_t>(i * 3u);
  }
}

} // namespace

TEST_CASE("HTPA32_Eeprom serialize/deserialize roundtrip preserves data", "[HTPA32_Eeprom]") {
  HTPA32_Eeprom eeprom1{};
  fill_test_eeprom(eeprom1);

  std::array<std::uint8_t, HTPA32_Eeprom::SERIALIZED_SIZE> buffer{};

  SECTION("serialize writes full buffer size") {
    const auto written = eeprom1.serialize(buffer.data(), buffer.size());
    REQUIRE(written == buffer.size());
  }

  SECTION("roundtrip serialize/deserialize yields identical EEPROM contents") {
    const auto written = eeprom1.serialize(buffer.data(), buffer.size());
    REQUIRE(written == buffer.size());

    const auto maybe_eeprom2 = HTPA32_Eeprom::deserialize(buffer.data(), buffer.size());
    REQUIRE(maybe_eeprom2.has_value());
    const auto& eeprom2 = *maybe_eeprom2;

    const auto& d1 = eeprom1.data;
    const auto& d2 = eeprom2.data;

    REQUIRE(d2.pixc_min == Catch::Approx(d1.pixc_min));
    REQUIRE(d2.pixc_max == Catch::Approx(d1.pixc_max));

    REQUIRE(d2.grad_scale == d1.grad_scale);
    REQUIRE(d2.tablenumber == d1.tablenumber);

    REQUIRE(d2.epsilon == d1.epsilon);
    REQUIRE(d2.mbit_calib == d1.mbit_calib);
    REQUIRE(d2.bias_calib == d1.bias_calib);
    REQUIRE(d2.clk_calib == d1.clk_calib);
    REQUIRE(d2.bpa_calib == d1.bpa_calib);
    REQUIRE(d2.pu_calib == d1.pu_calib);
    REQUIRE(d2.arraytype == d1.arraytype);

    REQUIRE(d2.vddth1 == d1.vddth1);
    REQUIRE(d2.vddth2 == d1.vddth2);

    REQUIRE(d2.ptat_gradient == Catch::Approx(d1.ptat_gradient));
    REQUIRE(d2.ptat_offset == Catch::Approx(d1.ptat_offset));

    REQUIRE(d2.ptat_th1 == d1.ptat_th1);
    REQUIRE(d2.ptat_th2 == d1.ptat_th2);

    REQUIRE(d2.vddsc_gradient == d1.vddsc_gradient);
    REQUIRE(d2.vddsc_offset == d1.vddsc_offset);
    REQUIRE(d2.global_offset == d1.global_offset);

    REQUIRE(d2.global_gain == d1.global_gain);

    REQUIRE(d2.mbit_user == d1.mbit_user);
    REQUIRE(d2.bias_user == d1.bias_user);
    REQUIRE(d2.clk_user == d1.clk_user);
    REQUIRE(d2.bpa_user == d1.bpa_user);
    REQUIRE(d2.pu_user == d1.pu_user);

    REQUIRE(d2.device_id == d1.device_id);
    REQUIRE(d2.norof_deadpix == d1.norof_deadpix);

    for (int i = 0; i < 24; ++i) {
      REQUIRE(d2.deadpix_addr[i] == d1.deadpix_addr[i]);
    }
    for (int i = 0; i < 12; ++i) {
      REQUIRE(d2.deadpix_mask[i] == d1.deadpix_mask[i]);
    }

    for (int i = 0; i < 256; ++i) {
      REQUIRE(d2.vddcomp_gradient[i] == d1.vddcomp_gradient[i]);
      REQUIRE(d2.vddcomp_offset[i] == d1.vddcomp_offset[i]);
    }

    for (int i = 0; i < 1024; ++i) {
      REQUIRE(d2.th_gradient[i] == d1.th_gradient[i]);
      REQUIRE(d2.th_offset[i] == d1.th_offset[i]);
      REQUIRE(d2.p[i] == d1.p[i]);
    }
  }
}

TEST_CASE("HTPA32_Eeprom primitive field encoding/decoding matches expected bytes", "[HTPA32_Eeprom]") {
  // Use known bit patterns for floats and integers to verify the low-level
  // serialization helpers (write/read of u8/u16/u32/float) used by the C layer.

  // Helper union to control the exact float bit pattern without requiring C++20.
  union U32F {
    std::uint32_t u;
    float f;
  };

  HTPA32_Eeprom eeprom{};
  auto& d = eeprom.data;

  // Values taken from the MCU example in the bug report.
  U32F pixc_min_bits{ 0x4BAC2819u };
  U32F pixc_max_bits{ 0x4CE8DF46u };

  d.pixc_min = pixc_min_bits.f;
  d.pixc_max = pixc_max_bits.f;

  d.grad_scale  = 0x16u;
  d.tablenumber = 0x0072u;

  d.epsilon    = 0x64u;
  d.mbit_calib = 0x2Cu;
  d.bias_calib = 0x05u;
  d.clk_calib  = 0x15u;
  d.bpa_calib  = 0x03u;

  std::array<std::uint8_t, HTPA32_Eeprom::SERIALIZED_SIZE> buffer{};
  const auto written = eeprom.serialize(buffer.data(), buffer.size());
  REQUIRE(written == buffer.size());

  // Check the first bytes in detail. Layout is little-endian:
  //  - pixc_min  (4 bytes)
  //  - pixc_max  (4 bytes)
  //  - grad_scale (1 byte)
  //  - tablenumber (2 bytes, LE)
  //  - epsilon, mbit_calib, bias_calib, clk_calib, bpa_calib (1 byte each)
  const std::array<std::uint8_t, 16> expected_prefix{
    0x19, 0x28, 0xAC, 0x4B, // pixc_min  = 0x4BAC2819
    0x46, 0xDF, 0xE8, 0x4C, // pixc_max  = 0x4CE8DF46
    0x16,                   // grad_scale
    0x72, 0x00,             // tablenumber (0x0072 LE)
    0x64,                   // epsilon
    0x2C,                   // mbit_calib
    0x05,                   // bias_calib
    0x15,                   // clk_calib
    0x03                    // bpa_calib
  };

  for (std::size_t i = 0; i < expected_prefix.size(); ++i) {
    INFO("Byte index: " << i);
    REQUIRE(buffer[i] == expected_prefix[i]);
  }

  // Now deserialize from the same bytes and ensure we recover the original values.
  // We create a fresh buffer initialized with exactly the expected prefix plus zeros.
  std::array<std::uint8_t, HTPA32_Eeprom::SERIALIZED_SIZE> buffer2{};
  std::copy(expected_prefix.begin(), expected_prefix.end(), buffer2.begin());

  auto maybe_eeprom2 = HTPA32_Eeprom::deserialize(buffer2.data(), buffer2.size());
  REQUIRE(maybe_eeprom2.has_value());
  const auto& eeprom2 = *maybe_eeprom2;
  const auto& d2      = eeprom2.data;

  // Compare float bit patterns exactly rather than with Approx.
  U32F pixc_min_bits2{};
  pixc_min_bits2.f = d2.pixc_min;
  U32F pixc_max_bits2{};
  pixc_max_bits2.f = d2.pixc_max;

  REQUIRE(pixc_min_bits2.u == pixc_min_bits.u);
  REQUIRE(pixc_max_bits2.u == pixc_max_bits.u);

  REQUIRE(d2.grad_scale == d.grad_scale);
  REQUIRE(d2.tablenumber == d.tablenumber);
  REQUIRE(d2.epsilon == d.epsilon);
  REQUIRE(d2.mbit_calib == d.mbit_calib);
  REQUIRE(d2.bias_calib == d.bias_calib);
  REQUIRE(d2.clk_calib == d.clk_calib);
  REQUIRE(d2.bpa_calib == d.bpa_calib);
}
