// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example prints a depth map of the first connected ToF Sensor on the command line.
 * @date 2025-11-18
 */

#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/AnyDeviceParams.hpp>
#include <sensorring/device/depth/DepthSensor.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"

using namespace eduart::sensorring;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SocketCan;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::UsbTingo;

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

void printDepthMap(const measurement::DepthMeasurement& meas, bool reset_cursor) {

  if (reset_cursor) {
    std::cout << "\033[" << meas.resolution_y << "F";
  }

  for (unsigned int row = 0; row < meas.resolution_y; ++row) {
    for (unsigned int col = 0; col < meas.resolution_x; ++col) {
      int idx = row * meas.resolution_x + col;
      std::cout << depthToColor(meas.point_cloud.data[idx].raw_distance, MIN_DIST, MAX_DIST) << "██";
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

    // Create a SensorRing with one depth sensor board via auto-discovery
    SensorRingFactory factory;

    device::TMF8829_Params tmf_params;
    tmf_params.resolution_mode = device::ResolutionMode::RES_16X16;
    factory.setDefaultDeviceParams(tmf_params);

    factory.addInterface(can_interface);
    factory.expectBoard({}, { device::AnyDepthSensor_Params{} });
    factory.addInterface(usbtingo_interface);
    factory.expectBoard({}, { device::AnyDepthSensor_Params{} });

    // Create the MeasurementManager directly from the factory
    auto manager = std::make_unique<manager::MeasurementManager>(params, factory);

    // Subscribe to the state changes to get the measurements
    auto state_sub = manager->subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "[State] State changed to: " << state << std::endl;
    });

    // Subscribe to the first depth sensor to get the measurements
    auto depth_sub = manager->depthSensors().subscribe([&got_first_measurement, &reset_cursor](const measurement::DepthMeasurement& meas) {
      got_first_measurement = true;
      printDepthMap(meas, reset_cursor);
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

      // Cancel subscriptions before stopping (optional — destruction also cancels)
      state_sub.cancel();
      depth_sub.cancel();
      log_sub.cancel();

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}