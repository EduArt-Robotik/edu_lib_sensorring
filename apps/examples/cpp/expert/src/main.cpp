// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  Expert example demonstrating expert-user features of the SensorRing library.
 *
 *         This example shows how to:
 *         - Enumerate hardware before building the SensorRing
 *         - Build the SensorRing manually and pass it to the MeasurementManager
 *         - Configure per-board poses and per-device params via the factory
 *         - Use per-device subscriptions alongside group subscriptions
 *         - Form custom spatial subgroups from selected devices
 *         - Control the measurement loop manually with measureSome()
 *         - Monitor state changes for diagnostics
 *
 * @date 2026-04-21
 */

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/depth/DepthSensor.hpp>
#include <sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp>
#include <sensorring/device/light/Light.hpp>
#include <sensorring/device/light/ws2812b/WS2812b_Params.hpp>
#include <sensorring/device/thermal/ThermalSensor.hpp>
#include <sensorring/device/thermal/htpa32/HTPA32_Params.hpp>
#include <sensorring/device/types/Group.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>
#include <vector>

using namespace eduart::sensorring;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SocketCan;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::UsbTingo;

int main(int, char*[]) {
  std::cout << "================================" << std::endl;
  std::cout << "Expert sensorring example" << std::endl;
  std::cout << "================================" << std::endl;
  std::cout << std::endl;

  // --- Logger ---
  auto log_sub = logger::Logger::getInstance()->subscribe([](const logger::LogVerbosity verbosity, const std::string& msg) {
    if (verbosity > logger::LogVerbosity::Debug)
      std::cout << "[" << verbosity << "] " << msg << std::endl;
  });

  try {
    // =========================================================================
    // 1. Configure interfaces
    // =========================================================================
    com::ComInterfaceID can_interface;
    can_interface.type = CAN_INTERFACE_TYPE;
    can_interface.name = CAN_INTERFACE_NAME;

    com::ComInterfaceID usbtingo_interface;
    usbtingo_interface.type = USBTINGO_INTERFACE_TYPE;
    usbtingo_interface.name = USBTINGO_INTERFACE_NAME;

    // =========================================================================
    // 2. Configure the factory with explicit board poses and device params
    // =========================================================================

    // Use Relaxed validation so that the build succeeds even if not all
    // expected boards are physically present.
    ring::SensorRingFactory factory(ring::ValidationMode::Relaxed);

    factory.addInterface(can_interface);

    // Board 0: Front-left, rotated 45° around Z.
    // Explicitly request ToF + thermal + LED with custom params.
    device::VL53L8CX_Params vl53_params;
    device::HTPA32_Params htpa_params;
    htpa_params.auto_min_max = true;
    device::WS2812b_Params ws_params;

    device::SensorBoardParams board_0;
    board_0.rotation    = { 0, 0, 45 };
    board_0.translation = { 0.1, 0.05, 0 };
    factory.expectBoard(board_0, { vl53_params, htpa_params, ws_params });

    // Board 1: Front-right, rotated -45° around Z.
    device::SensorBoardParams board_1;
    board_1.rotation    = { 0, 0, -45 };
    board_1.translation = { 0.1, -0.05, 0 };
    factory.expectBoard(board_1, { vl53_params, htpa_params, ws_params });

    // Second interface (if available).
    factory.addInterface(usbtingo_interface);
    device::SensorBoardParams board_2;
    board_2.rotation    = { 0, 0, 0 };
    board_2.translation = { -0.1, 0, 0 };
    factory.expectBoard(board_2, { vl53_params, htpa_params, ws_params });

    // =========================================================================
    // 3. Enumerate hardware (optional — useful for diagnostics)
    // =========================================================================
    auto topology = factory.enumerate();
    std::cout << "\n--- Hardware Topology ---\n" << factory.printTopology() << std::endl;

    // =========================================================================
    // 4. Build the SensorRing manually and hand it to the MeasurementManager
    //    (expert-user constructor taking unique_ptr<SensorRing>)
    // =========================================================================
    auto sensor_ring = factory.build();
    if (!sensor_ring) {
      std::cerr << "Factory build() failed — no compatible hardware found." << std::endl;
      return EXIT_FAILURE;
    }

    manager::ManagerParams params;
    params.frequency_tof_hz     = 10.0; // Cap ToF rate to 10 Hz
    params.frequency_thermal_hz = 4.0;  // Cap thermal rate to 4 Hz
    params.repair_errors        = true;

    // Expert-user constructor: takes ownership of the pre-built SensorRing.
    manager::MeasurementManager manager(params, std::move(sensor_ring));

    // =========================================================================
    // 5. Retrieve typed device groups
    // =========================================================================
    auto all_depth   = manager.depthSensors();
    auto all_thermal = manager.thermalSensors();
    auto all_lights  = manager.lights();

    std::cout << "\nDiscovered devices:\n"
              << "  Depth sensors:   " << all_depth.size() << "\n"
              << "  Thermal sensors: " << all_thermal.size() << "\n"
              << "  Lights:          " << all_lights.size() << "\n"
              << std::endl;

    // =========================================================================
    // 6. Per-device subscriptions (each sensor gets its own callback)
    // =========================================================================
    std::vector<subscription::Subscription> per_device_subs;

    for (std::size_t i = 0; i < all_depth.size(); ++i) {
      per_device_subs.push_back(all_depth[i].subscribe([i](const measurement::DepthMeasurement& m) {
        // Per-sensor processing — e.g. obstacle detection for a specific sector.
        if (m.nr_valid_points > 0) {
          // Access the transformed (world-frame) point cloud:
          // m.transformed_point_cloud contains points rotated/translated
          // according to the board pose set via expectBoard().
        }
      }));
    }

    // =========================================================================
    // 7. Custom spatial subgroup (if we have at least 2 depth sensors)
    //    Group a subset of sensors for region-specific processing.
    // =========================================================================
    subscription::Subscription front_sub;
    if (all_depth.size() >= 2) {
      // Create a group from the first two depth sensors ("front" region).
      auto front_depth = device::Group<device::DepthSensor>({ &all_depth[0], &all_depth[1] });

      front_sub = front_depth.subscribe([](const measurement::DepthMeasurement& /*m*/) {
        // Fires for each sensor in the subgroup, once per measurement cycle.
        // Combine them for a wider front FOV, run collision checks, etc.
      });

      std::cout << "Front depth subgroup: " << front_depth.size() << " sensors\n";
    }

    // =========================================================================
    // 8. Group subscription for thermal (per-sensor callbacks)
    // =========================================================================
    std::atomic<unsigned int> thermal_frame_count{ 0 };
    auto thermal_sub = all_thermal.subscribe([&thermal_frame_count](const measurement::ThermalMeasurement& m) {
      if (m.sensor_index == 0) {
        thermal_frame_count++;
      }
      // m.temperatures holds the 32×32 temperature array in °C.
      // m.min_deg_c / m.max_deg_c give the frame extremes.
    });

    // =========================================================================
    // 9. State change monitoring
    // =========================================================================
    auto state_sub = manager.subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "[State] " << state << std::endl;
    });

    // =========================================================================
    // 10. Set initial light state via the action queue
    // =========================================================================
    for (auto& light : all_lights) {
      light.setMode(device::LightMode::Pulsation);
    }

    // =========================================================================
    // 11a. Option A: Threaded measurement loop (most common)
    // =========================================================================
    manager.startMeasuring();

    auto start = std::chrono::steady_clock::now();
    while (manager.isMeasuring() && (std::chrono::steady_clock::now() - start < 10s)) {
      std::cout << "Thermal frames: " << thermal_frame_count.load() << "\r" << std::flush;
      std::this_thread::sleep_for(1s);
    }

    // Switch lights to fixed green before shutdown.
    for (auto& light : all_lights) {
      light.setMode(device::LightMode::FixedColor);
      light.setColor(0, 128, 0);
    }
    std::this_thread::sleep_for(500ms); // Give state machine one cycle to drain the queue.

    manager.stopMeasuring();

    // =========================================================================
    // 11b. Option B: Manual measurement loop (commented out)
    //      Gives full control over timing — useful for synchronising with
    //      external systems or running inside a game/physics loop.
    // =========================================================================
    // while (running) {
    //   manager.measureSome();   // drives one state machine cycle
    //   myPhysicsStep();
    // }

    // =========================================================================
    // 12. Clean shutdown — subscriptions cancel automatically via RAII,
    //     but explicit cancellation is also supported:
    // =========================================================================
    thermal_sub.cancel();
    front_sub.cancel();
    for (auto& s : per_device_subs)
      s.cancel();
    state_sub.cancel();
    log_sub.cancel();

    std::cout << "\nTotal thermal frames received: " << thermal_frame_count.load() << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Caught: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
