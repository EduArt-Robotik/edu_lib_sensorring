#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/device/DeviceGroup.hpp"
#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/subscription/Subscription.hpp"

using eduart::sensorring::com::InterfaceType;
using eduart::sensorring::manager::ManagerParams;
using eduart::sensorring::manager::MeasurementManager;
using eduart::sensorring::measurement::TofMeasurement;

namespace {

enum class TestResult {
  NotAvailable,
  Passed,
  Failed
};

TestResult run_single_interface_test(const std::string& interface_name, InterfaceType type) {
  ManagerParams params;

  eduart::sensorring::com::ComInterfaceID interface;
  interface.type = type;
  interface.name = interface_name;

  std::vector<TofMeasurement> measurements;
  std::size_t count = 0;

  try {
    eduart::sensorring::ring::SensorRingFactory factory;
    factory.addInterface(interface);
    factory.expectBoard({}, { eduart::sensorring::device::VL53L8CX_Params() });
    auto sensor_ring = factory.build(eduart::sensorring::ring::ValidationMode::Relaxed);

    if (!sensor_ring) {
      return TestResult::NotAvailable;
    }

    MeasurementManager manager(params, std::move(sensor_ring));

    auto sub = manager.subscribeToDeviceGroup(eduart::sensorring::device::DeviceType::VL53L8CX, [&measurements, &count](const eduart::sensorring::device::DeviceGroup& group) {
      group.invokeForEachDeviceOfType<eduart::sensorring::device::VL53L8CX_Device>([&measurements, &count](eduart::sensorring::device::VL53L8CX_Device* device) {
        if (!device->getEnable())
          return;
        auto [meas, state] = device->getLatestMeasurement();
        if (state == eduart::sensorring::device::DeviceState::Ok && !meas.point_cloud.data.empty()) {
          measurements.push_back(meas);
          count++;
        }
      });
    });

    if (!manager.startMeasuring()) {
      manager.stopMeasuring();
      return TestResult::NotAvailable;
    }

    // Wait up to ~5 seconds for up to 3 frames.
    const auto start = std::chrono::steady_clock::now();
    while (count < 3 && std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    manager.stopMeasuring();

    if (count == 0) {
      // No measurements received – likely no active ToF on this interface.
      return TestResult::NotAvailable;
    }

    const auto& first = measurements.front();

    const auto& points = first.point_cloud.data;
    if (points.empty()) {
      return TestResult::Failed;
    }

    // Heuristic 1: at least half of the points have non-zero distance.
    std::size_t non_zero_count = 0;
    for (const auto& p : points) {
      if (p.raw_distance > 0.0) {
        non_zero_count++;
      }
    }

    if (non_zero_count * 2 < points.size()) {
      return TestResult::Failed;
    }

    // Heuristic 2: if we observed at least two frames, frame_id should change.
    if (count >= 2) {
      const auto& second = measurements[1];
      if (second.frame_id == first.frame_id) {
        return TestResult::Failed;
      }
    }

    return TestResult::Passed;
  } catch (...) {
    // Any construction/communication error on this interface means "not available" for our purposes.
    return TestResult::NotAvailable;
  }
}

} // namespace

// NOTE:
// These tests require real hardware connected and appropriately configured.
// They are intended to be enabled explicitly via SENSORRING_BUILD_HARDWARE_TESTS.

TEST_CASE("ToF hardware end-to-end measurement via USBtingo or SocketCAN with a single board", "[ToFHardware]") {
  // First try USBtingo with serial "0" (first connected device).
  TestResult usbtingo_result = run_single_interface_test("0", InterfaceType::UsbTingo);

  if (usbtingo_result == TestResult::Passed) {
    SUCCEED("ToF measurement via USBtingo(0) is plausible.");
    return;
  }

  // Fallback: try SocketCAN on interface "can0".
  TestResult socketcan_result = run_single_interface_test("can0", InterfaceType::SocketCan);

  if (socketcan_result == TestResult::Passed) {
    SUCCEED("ToF measurement via SocketCAN(can0) is plausible.");
    return;
  }

  // Only fail the test when an interface was reachable but produced implausible data.
  REQUIRE_FALSE(usbtingo_result == TestResult::Failed);
  REQUIRE_FALSE(socketcan_result == TestResult::Failed);

  // If both are NotAvailable, treat as effectively skipped (but still report a warning).
  FAIL("No usable ToF interface found (neither USBtingo(0) nor SocketCAN(can0)).");
}
