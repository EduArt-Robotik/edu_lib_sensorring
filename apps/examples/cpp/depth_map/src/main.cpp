// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example prints a depth map of the first connected ToF Sensor on the command line.
 * @date 2025-11-18
 */

#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

using namespace eduart;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SOCKETCAN;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::USBTINGO;

// Distance range for color mapping (in meters)
static constexpr double MIN_DIST = 0.0;
static constexpr double MAX_DIST = 1.0;

std::string depthToColor(double depth, double min, double max) {

  if (depth > max || depth < 0) {
    depth = max;
  } else if (depth < min) {
    depth = min;
  }

  auto d = (depth - min) / (max - min);

  // Map t to a color gradient: red (near) → yellow → green → blue (far)
  int r = static_cast<int>(255 * (1 - d));
  int g = static_cast<int>(255 * (1 - std::abs(0.5f - d) * 2));
  int b = static_cast<int>(255 * d);

  return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

void printDepthMap(const measurement::PointCloud& points, bool reset_cursor) {

  if (reset_cursor) {
    std::cout << "\033[8F";
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      int idx = row * 8 + col;
      std::cout << depthToColor(points.data[idx].raw_distance, MIN_DIST, MAX_DIST) << "██";
    }
    std::cout << "\033[0m\n";
  }

  std::cout.flush();
  reset_cursor = true;
}

int main(int, char*[]) {

  std::cout << "\33c";
  std::cout << "============================" << std::endl;
  std::cout << "Depth map sensorring example" << std::endl;
  std::cout << "============================" << std::endl;
  std::cout << std::endl;

  std::atomic<bool> reset_cursor          = false;
  std::atomic<bool> got_first_measurement = false;

  manager::ManagerParams params;

  com::ComInterfaceID can_interface;
  can_interface.type = CAN_INTERFACE_TYPE;
  can_interface.name = CAN_INTERFACE_NAME;

  com::ComInterfaceID usbtingo_interface;
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE;
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME;

  try {
    // Subscribe to the log messages
    auto log_sub = logger::Logger::getInstance()->subscribe([&reset_cursor](const logger::LogVerbosity verbosity, const std::string& msg) {
      if (verbosity > logger::LogVerbosity::Debug) {
        std::cout << "[" << verbosity << "] " << msg << std::endl;
        reset_cursor = false;
      }
    });

    // Create a SensorRing with one VL53L8CX board via auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.addInterface(usbtingo_interface);
    factory.expectBoard({}, { device::VL53L8CX_Params{} });
    auto sensor_ring = factory.build(ring::ValidationMode::Relaxed);

    if (!sensor_ring) {
      std::cout << "Failed to create SensorRing. Exiting." << std::endl;
      return 1;
    }

    // Create the MeasurementManager with the SensorRing
    auto manager = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));

    // Subscribe to the state changes to get the measurements
    auto state_sub = manager->subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "[State] State changed to: " << state << std::endl;
    });

    // Subscribe to the VL53L8CX device group to get the measurements
    auto vl53l8cx_sub = manager->subscribeToDeviceGroup(device::DeviceType::VL53L8CX, [&got_first_measurement, &reset_cursor](const device::DeviceGroup& devs) {
      got_first_measurement = true;
      auto vl53l8cx         = devs.getDevicesOfType<device::VL53L8CX_Device>().at(0);
      printDepthMap(vl53l8cx->getLatestRawMeasurement().first.point_cloud, reset_cursor);
      reset_cursor = true;
    });

    // Start the measurements
    manager->startMeasuring();

    while (!got_first_measurement && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {

      while (manager->isMeasuring()) {
        std::this_thread::sleep_for(1s);
      }

      // Unsubscribe from manager and logger before stopping (optional)
      manager->unsubscribe(state_sub);
      manager->unsubscribe(vl53l8cx_sub);
      logger::Logger::getInstance()->unsubscribe(log_sub);

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}