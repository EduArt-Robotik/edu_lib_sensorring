#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"

using eduart::sensorring::SensorRingFactory;
using eduart::sensorring::com::InterfaceType;
using eduart::sensorring::device::ResolutionMode;
using eduart::sensorring::device::TMF8829_Device;
using eduart::sensorring::device::TMF8829_Params;
using eduart::sensorring::device::TMF8829_ResultFormat;

namespace {

/**
 * @brief Try to open an interface and return the first TMF8829_Device found.
 *
 * @return Non-owning pointer to the device (owned by the ring), or nullptr if
 *         the interface is not available or no TMF8829 is connected.
 */
struct HardwareContext {
  std::unique_ptr<eduart::sensorring::SensorRing> ring;
  TMF8829_Device* device = nullptr;
};

HardwareContext open_tmf8829(const std::string& interface_name, InterfaceType type) {
  HardwareContext ctx;

  try {
    SensorRingFactory factory;

    eduart::sensorring::com::ComInterfaceID interface;
    interface.type = type;
    interface.name = interface_name;

    factory.addInterface(interface);
    factory.expectBoard({}, { TMF8829_Params() });

    ctx.ring = factory.build();
    if (!ctx.ring) {
      return ctx;
    }

    for (auto* base : ctx.ring->getDevices()) {
      auto* tmf = dynamic_cast<TMF8829_Device*>(base);
      if (tmf) {
        ctx.device = tmf;
        break;
      }
    }
  } catch (...) {
    ctx.ring.reset();
    ctx.device = nullptr;
  }

  return ctx;
}

} // namespace

// NOTE:
// These tests require a single TMF8829 sensor board connected via USBtingo or SocketCAN.
// Enable via the SENSORRING_BUILD_HARDWARE_TESTS CMake option.

TEST_CASE("TMF8829 hardware parameter get/set round-trip", "[TMF8829Hardware]") {
  // Try USBtingo first, then fall back to SocketCAN.
  HardwareContext ctx = open_tmf8829("0", InterfaceType::UsbTingo);

  if (!ctx.device) {
    ctx = open_tmf8829("can0", InterfaceType::SocketCan);
  }

  if (!ctx.device) {
    FAIL("No TMF8829 device found on USBtingo(0) or SocketCAN(can0). Ensure one board with a TMF8829 is connected.");
  }

  TMF8829_Device& dev = *ctx.device;

  // -------------------------------------------------------------------------
  SECTION("Resolution modes – all modes can be set and read back") {
    const ResolutionMode modes[] = {
      ResolutionMode::RES_8X8,
      ResolutionMode::RES_8X8_LONG_RANGE,
      ResolutionMode::RES_8X8_HIGH_ACCURACY,
      ResolutionMode::RES_16X16,
      ResolutionMode::RES_16X16_HIGH_ACCURACY,
      ResolutionMode::RES_32X32,
      ResolutionMode::RES_32X32_HIGH_ACCURACY,
      ResolutionMode::RES_48X32,
      ResolutionMode::RES_48X32_HIGH_ACCURACY,
    };

    for (const auto mode : modes) {
      REQUIRE(dev.setResolutionMode(mode));

      ResolutionMode readback = ResolutionMode::RES_8X8;
      REQUIRE(dev.getResolutionMode(readback));
      REQUIRE(readback == mode);
    }

    // Leave the sensor in the default mode.
    REQUIRE(dev.setResolutionMode(ResolutionMode::RES_8X8));
  }

  // -------------------------------------------------------------------------
  SECTION("full_noise flag – set true then false") {
    REQUIRE(dev.setResultFullNoise(true));
    bool value = false;
    REQUIRE(dev.getResultFullNoise(value));
    REQUIRE(value == true);

    REQUIRE(dev.setResultFullNoise(false));
    REQUIRE(dev.getResultFullNoise(value));
    REQUIRE(value == false);
  }

  // -------------------------------------------------------------------------
  SECTION("xtalk flag – set true then false") {
    REQUIRE(dev.setResultXtalk(true));
    bool value = false;
    REQUIRE(dev.getResultXtalk(value));
    REQUIRE(value == true);

    REQUIRE(dev.setResultXtalk(false));
    REQUIRE(dev.getResultXtalk(value));
    REQUIRE(value == false);
  }

  // -------------------------------------------------------------------------
  SECTION("noise_strength flag – set true then false") {
    REQUIRE(dev.setResultNoiseStrength(true));
    bool value = false;
    REQUIRE(dev.getResultNoiseStrength(value));
    REQUIRE(value == true);

    REQUIRE(dev.setResultNoiseStrength(false));
    REQUIRE(dev.getResultNoiseStrength(value));
    REQUIRE(value == false);
  }

  // -------------------------------------------------------------------------
  SECTION("signal_strength flag – set true then false") {
    REQUIRE(dev.setResultSignalStrength(true));
    bool value = false;
    REQUIRE(dev.getResultSignalStrength(value));
    REQUIRE(value == true);

    REQUIRE(dev.setResultSignalStrength(false));
    REQUIRE(dev.getResultSignalStrength(value));
    REQUIRE(value == false);
  }

  // -------------------------------------------------------------------------
  SECTION("nr_of_peaks – all valid values 0..4") {
    for (std::uint8_t peaks = 0; peaks <= 4; ++peaks) {
      REQUIRE(dev.setResultNrOfPeaks(peaks));
      std::uint8_t readback = 255u;
      REQUIRE(dev.getResultNrOfPeaks(readback));
      REQUIRE(readback == peaks);
    }

    // Restore default.
    REQUIRE(dev.setResultNrOfPeaks(1));
  }

  // -------------------------------------------------------------------------
  SECTION("result format round-trip via setResultFormat / getResultFormat") {
    TMF8829_ResultFormat fmt;
    fmt.full_noise      = true;
    fmt.xtalk           = true;
    fmt.noise_strength  = true;
    fmt.signal_strength = true;
    fmt.nr_of_peaks     = 3;

    REQUIRE(dev.setResultFormat(fmt));

    TMF8829_ResultFormat readback{};
    REQUIRE(dev.getResultFormat(readback));
    REQUIRE(readback.full_noise == fmt.full_noise);
    REQUIRE(readback.xtalk == fmt.xtalk);
    REQUIRE(readback.noise_strength == fmt.noise_strength);
    REQUIRE(readback.signal_strength == fmt.signal_strength);
    REQUIRE(readback.nr_of_peaks == fmt.nr_of_peaks);

    // Restore defaults.
    REQUIRE(dev.setResultFormat(TMF8829_ResultFormat{}));
  }
}
