// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example prints a depth map of the first connected ToF Sensor on the command line.
 * @date 2025-11-18
 */

#include <chrono>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/hardware/htpa32/HTPA32_Device.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

using namespace eduart;
using namespace std::chrono_literals;

static constexpr std::string_view INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType INTERFACE_TYPE = com::InterfaceType::SOCKETCAN;

std::string colorStringCommand(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

void printFalseColorImage(const measurement::FalseColorImage& img, bool reset_cursor) {

  if (reset_cursor) {
    std::cout << "\033[32F";
  }

  for (int row = 0; row < 32; ++row) {
    for (int col = 0; col < 32; ++col) {
      int idx = row * 32 + col;
      std::cout << colorStringCommand(img.data[idx][0], img.data[idx][1], img.data[idx][2]) << "██";
      // std::cout << colorStringCommand(row, row, row) << "██";
    }
    std::cout << "\033[0m\n";
  }

  std::cout.flush();
}

int main(int, char*[]) {

  std::cout << "\33c";
  std::cout << "============================" << std::endl;
  std::cout << "Depth map sensorring example" << std::endl;
  std::cout << "============================" << std::endl;
  std::cout << std::endl;

  std::atomic<bool> reset_cursor = false;

  // Create the parameter structure that is used to instantiate the sensorring
  manager::ManagerParams params;
  params.timeout              = 1h;
  params.frequency_thermal_hz = 5.0;

  com::ComInterfaceID interface;
  interface.type = INTERFACE_TYPE;
  interface.name = INTERFACE_NAME;

  ring::SensorRingFactory factory;

  try {
    // Subscribe to the log messages
    auto log_sub = logger::Logger::getInstance()->subscribe([&reset_cursor](const logger::LogVerbosity verbosity, const std::string& msg) {
      // if (verbosity > logger::LogVerbosity::Debug)
      std::cout << "[" << verbosity << "] " << msg << std::endl;
      reset_cursor = false;
    });

    // Create the SensorRing via auto-discovery
    factory.addInterface(interface);
    factory.expectBoard({}, { device::HTPA32_Params{} });
    auto sensor_ring = factory.build(ring::ValidationMode::Relaxed);

    if (!sensor_ring) {
      std::cout << "Failed to create SensorRing from enumeration. Exiting example application." << std::endl;
      return 1;
    }

    // Create the MeasurementManager with the SensorRing
    auto manager = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));

    // Subscribe to the state changes to get the measurements
    auto state_sub = manager->subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "[State] State changed to: " << state << std::endl;
    });

    std::atomic<bool> got_first_measurement = false;
    // Subscribe to the Thermal device group to get the measurements
    auto htpa32_sub = manager->subscribeToDeviceGroup(device::DeviceType::HTPA32, [&got_first_measurement, &reset_cursor](const device::DeviceGroup& devs) {
      got_first_measurement = true;
      auto htpa32           = devs.getDevicesOfType<device::HTPA32_Device>().at(0);
      printFalseColorImage(htpa32->getLatestFalseColorImage().first, reset_cursor);
      reset_cursor = true;
    });

    // Start the measurements
    manager->startMeasuring();

    while (!got_first_measurement && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {

      manager->enqueueExtraAction([&manager]() {
        auto devs   = device::DeviceGroup(manager->getSensorRing()->getDevices());
        auto htpa32 = devs.getDevicesOfType<device::HTPA32_Device>().at(0);
        std::cout << "Starting calibration of thermal sensors." << std::endl;
        htpa32->startCalibration(20);
      });

      while (manager->isMeasuring()) {
        std::this_thread::sleep_for(1s);
      }

      // Stop the measurements
      manager->unsubscribe(state_sub);
      manager->unsubscribe(htpa32_sub);
      manager->stopMeasuring();

      logger::Logger::getInstance()->unsubscribe(log_sub);
    }

  } catch (const std::exception& e) {
    std::cout << "Caught exception in example application: " << e.what() << std::endl;
  }

  return 0;
}