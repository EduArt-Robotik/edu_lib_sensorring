// Unit tests for eduart::manager::MeasurementClient (observer interface for MeasurementManager).

#include <catch2/catch_all.hpp>
#include <cstdint>
#include <vector>

#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/manager/MeasurementClient.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"
#include "sensorring/measurement/TofMeasurement.hpp"

using eduart::manager::ManagerState;
using eduart::manager::MeasurementClient;
using eduart::manager::MeasurementManager;
using eduart::measurement::ThermalMeasurement;
using eduart::measurement::TofMeasurement;

// Client subclass that records callback invocations for testing.
struct RecordingClient : MeasurementClient {
  void onStateChange(const ManagerState state) override { state_changes.push_back(state); }
  void onRawTofMeasurement(const std::vector<TofMeasurement>& measurement_vec) override {
    raw_tof_calls++;
    raw_tof_last_size = measurement_vec.size();
  }
  void onTransformedTofMeasurement(const std::vector<TofMeasurement>& measurement_vec) override {
    transformed_tof_calls++;
    transformed_tof_last_size = measurement_vec.size();
  }
  void onThermalMeasurement(const std::vector<ThermalMeasurement>& measurement_vec) override {
    thermal_calls++;
    thermal_last_size = measurement_vec.size();
  }

  std::vector<ManagerState> state_changes;
  int raw_tof_calls                     = 0;
  std::size_t raw_tof_last_size         = 0;
  int transformed_tof_calls             = 0;
  std::size_t transformed_tof_last_size = 0;
  int thermal_calls                     = 0;
  std::size_t thermal_last_size         = 0;
};

// Sentinel pointer used only for testing unregisterClient(ptr) when ptr is not in the client's set.
// We never dereference it; we only pass it to unregisterClient to verify erase returns false.
static MeasurementManager* const kUnregisteredManager = reinterpret_cast<MeasurementManager*>(static_cast<uintptr_t>(1));

TEST_CASE("MeasurementClient registerClient", "[MeasurementClient]") {
  SECTION("registerClient with null returns false") {
    MeasurementClient client;
    REQUIRE_FALSE(client.registerClient(nullptr));
  }
}

TEST_CASE("MeasurementClient unregisterClient", "[MeasurementClient]") {
  SECTION("unregisterClient with null returns false") {
    MeasurementClient client;
    REQUIRE_FALSE(client.unregisterClient(nullptr));
  }

  SECTION("unregisterClient(manager) when not registered returns false") {
    MeasurementClient client;
    REQUIRE_FALSE(client.unregisterClient(kUnregisteredManager));
  }
}

TEST_CASE("MeasurementClient callback overrides", "[MeasurementClient]") {
  SECTION("default implementation does not crash when callbacks are invoked") {
    MeasurementClient client;
    client.onStateChange(ManagerState::Running);
    std::vector<TofMeasurement> tof_vec;
    client.onRawTofMeasurement(tof_vec);
    client.onTransformedTofMeasurement(tof_vec);
    std::vector<ThermalMeasurement> thermal_vec;
    client.onThermalMeasurement(thermal_vec);
  }

  SECTION("overridden callbacks are invoked") {
    RecordingClient client;
    client.onStateChange(ManagerState::Initialized);
    client.onStateChange(ManagerState::Running);
    REQUIRE(client.state_changes.size() == 2u);
    REQUIRE(client.state_changes[0] == ManagerState::Initialized);
    REQUIRE(client.state_changes[1] == ManagerState::Running);
  }

  SECTION("onRawTofMeasurement override is invoked") {
    RecordingClient client;
    std::vector<TofMeasurement> vec(3);
    client.onRawTofMeasurement(vec);
    REQUIRE(client.raw_tof_calls == 1);
    REQUIRE(client.raw_tof_last_size == 3u);
  }

  SECTION("onTransformedTofMeasurement override is invoked") {
    RecordingClient client;
    std::vector<TofMeasurement> vec(2);
    client.onTransformedTofMeasurement(vec);
    REQUIRE(client.transformed_tof_calls == 1);
    REQUIRE(client.transformed_tof_last_size == 2u);
  }

  SECTION("onThermalMeasurement override is invoked") {
    RecordingClient client;
    std::vector<ThermalMeasurement> vec(1);
    client.onThermalMeasurement(vec);
    REQUIRE(client.thermal_calls == 1);
    REQUIRE(client.thermal_last_size == 1u);
  }

  SECTION("callbacks with empty vectors") {
    RecordingClient client;
    std::vector<TofMeasurement> tof_empty;
    std::vector<ThermalMeasurement> thermal_empty;
    client.onRawTofMeasurement(tof_empty);
    client.onTransformedTofMeasurement(tof_empty);
    client.onThermalMeasurement(thermal_empty);
    REQUIRE(client.raw_tof_calls == 1);
    REQUIRE(client.raw_tof_last_size == 0u);
    REQUIRE(client.transformed_tof_calls == 1);
    REQUIRE(client.transformed_tof_last_size == 0u);
    REQUIRE(client.thermal_calls == 1);
    REQUIRE(client.thermal_last_size == 0u);
  }
}
