// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example prints a false-color thermal image of the first connected HTPA32 sensor on the command line.
 * @date 2025-11-18
 */

#include <chrono>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/ThermalSensor.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

using namespace eduart::sensorring;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SocketCan;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::UsbTingo;

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
  std::cout << "==============================" << std::endl;
  std::cout << "Thermal map sensorring example" << std::endl;
  std::cout << "==============================" << std::endl;
  std::cout << std::endl;

  std::atomic<bool> reset_cursor          = false;
  std::atomic<bool> got_first_measurement = false;

  manager::ManagerParams params;
  params.frequency_thermal_hz = 5.0;

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

    // Create a SensorRing with one HTPA32 board via auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.expectBoard({}, { device::HTPA32_Params{} });
    factory.addInterface(usbtingo_interface);
    factory.expectBoard({}, { device::HTPA32_Params{} });

    // Create the MeasurementManager directly from the factory
    auto manager = std::make_unique<manager::MeasurementManager>(params, factory);

    // Subscribe to the state changes to get the measurements
    auto state_sub = manager->subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "[State] State changed to: " << state << std::endl;
    });

    // Subscribe to the first thermal sensor to get the measurements
    auto thermal_sub = manager->thermalSensors().subscribe([&got_first_measurement, &reset_cursor](const measurement::ThermalMeasurement& meas) {
      got_first_measurement = true;
      printFalseColorImage(meas.temperatures.toFalseColor(), reset_cursor);
      reset_cursor = true;
    });

    // Start the measurements
    manager->startMeasuring();

    while (!got_first_measurement && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {

      // Start calibration directly via the ThermalSensor interface (thread-safe)
      std::cout << "Starting calibration of thermal sensors." << std::endl;
      for (auto& sensor : manager->thermalSensors()) {
        sensor.startCalibration(20);
      }

      while (manager->isMeasuring()) {
        std::this_thread::sleep_for(1s);
      }

      // Cancel subscriptions before stopping (optional — destruction also cancels)
      state_sub.cancel();
      thermal_sub.cancel();
      log_sub.cancel();

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}