#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "sensorring/MeasurementClient.hpp"
#include "sensorring/MeasurementManager.hpp"
#include "sensorring/types/InterfaceType.hpp"

using eduart::com::InterfaceType;
using eduart::manager::ManagerParams;
using eduart::manager::MeasurementManager;
using eduart::measurement::TofMeasurement;

namespace {

// Simple client that captures a few raw ToF measurements.
class TofCaptureClient : public eduart::manager::MeasurementClient {
public:
  void onRawTofMeasurement(const std::vector<TofMeasurement>& measurement_vec) override {
    if (measurement_vec.empty()) {
      return;
    }
    //if (_count >= _max_frames) {
    //  return;
    //}
    _measurements.push_back(measurement_vec.front());
    _count++;
  }

  std::size_t frameCount() const { return _count; }
  const std::vector<TofMeasurement>& frames() const { return _measurements; }

private:
  std::vector<TofMeasurement> _measurements;
  std::size_t _count{ 0 };
  //static constexpr std::size_t _max_frames = 5;
};

enum class TestResult {
  NotAvailable,
  Passed,
  Failed
};

TestResult run_single_interface_test(const std::string& interface_name, InterfaceType type) {
  // Configure one bus with a single ToF-enabled board, similar to minimal example.
  ManagerParams params;
  {
    eduart::sensor::TofSensorParams tof;
    tof.user_idx = 0;
    tof.enable   = true;

    eduart::sensor::SensorBoardParams board;
    board.tof_params = tof;

    eduart::bus::BusParams bus;
    bus.interface_name = interface_name;
    bus.type           = type;
    bus.board_param_vec.push_back(board);

    eduart::ring::RingParams ring;
    ring.bus_param_vec.push_back(bus);

    params.ring_params = ring;
  }

  TofCaptureClient client;

  try {
    MeasurementManager manager(params);
    manager.registerClient(&client);

    if (!manager.startMeasuring()) {
      manager.stopMeasuring();
      return TestResult::NotAvailable;
    }

    // Wait up to ~5 seconds for up to 3 frames.
    const auto start = std::chrono::steady_clock::now();
    while (client.frameCount() < 3 && std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    manager.stopMeasuring();

    if (client.frameCount() == 0) {
      // No measurements received – likely no active ToF on this interface.
      return TestResult::NotAvailable;
    }

    const auto& frames = client.frames();
    const auto& first  = frames.front();

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
    if (client.frameCount() >= 2) {
      const auto& second = frames[1];
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

TEST_CASE("ToF hardware end-to-end measurement via USBtingo or SocketCAN", "[ToFHardware][!mayfail]") {
  // First try USBtingo with serial "0" (first connected device).
  TestResult usbtingo_result = run_single_interface_test("0", InterfaceType::USBTINGO);

  if (usbtingo_result == TestResult::Passed) {
    SUCCEED("ToF measurement via USBtingo(0) is plausible.");
    return;
  }

  // Fallback: try SocketCAN on interface "can0".
  TestResult socketcan_result = run_single_interface_test("can0", InterfaceType::SOCKETCAN);

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
