// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   using_lambdas.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example receives measurements and prints the current measurement rate to the command line.
 * @date 2025-11-18
 */

#include <iomanip>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

#include "Rate.hpp"

using namespace eduart;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SocketCan;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::UsbTingo;

int main(int, char*[]) {
  std::cout << "==========================================" << std::endl;
  std::cout << "Minimal sensorring example (using lambdas)" << std::endl;
  std::cout << "==========================================" << std::endl;
  std::cout << std::endl;

  manager::ManagerParams params;

  auto vl53l8cx_rate = std::make_unique<Rate>();
  auto htpa32_rate   = std::make_unique<Rate>();

  com::ComInterfaceID can_interface;
  can_interface.type = CAN_INTERFACE_TYPE;
  can_interface.name = CAN_INTERFACE_NAME;

  com::ComInterfaceID usbtingo_interface;
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE;
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME;

  try {
    // Subscribe to the logger first so that all messages from initialization onward are captured.
    // Using a lambda here (rather than binding a class method) ensures the subscription is active
    // before any other object is constructed.
    auto log_sub = logger::Logger::getInstance()->subscribe([](const logger::LogVerbosity verbosity, const std::string& msg) {
      if (verbosity > logger::LogVerbosity::Debug)
        std::cout << "[" << verbosity << "] " << msg << std::endl;
    });

    // Create the SensorRing via auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.addInterface(usbtingo_interface);
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

    // Subscribe to the ToF device group to get the measurements
    auto vl53l8cx_sub = manager->subscribeToDeviceGroup(device::DeviceType::VL53L8CX, [&vl53l8cx_rate](const device::DeviceGroup& group) {
      vl53l8cx_rate->tick(group.getDeviceCount());
    });

    // Subscribe to the Thermal device group to get the measurements
    auto htpa32_sub = manager->subscribeToDeviceGroup(device::DeviceType::HTPA32, [&htpa32_rate](const device::DeviceGroup& group) {
      htpa32_rate->tick(group.getDeviceCount());
    });

    // Start the measurements
    manager->startMeasuring();

    while (!(vl53l8cx_rate->gotFirstMeasurement() || htpa32_rate->gotFirstMeasurement()) && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Start printing measurement rate." << std::endl;
      while (manager->isMeasuring()) {
        std::cout << "Current measurement rate: " << std::fixed << std::setprecision(2) << std::setw(5) << vl53l8cx_rate->getRate() << " Hz (ToF) from " << vl53l8cx_rate->getSensorCount() << " sensors, " << std::setw(5)
                  << htpa32_rate->getRate() << " Hz (Thermal) from " << htpa32_rate->getSensorCount() << " sensors\r" << std::flush;
        std::this_thread::sleep_for(1s);
      }

      // Cancel subscriptions before stopping (optional — destruction also cancels)
      state_sub.cancel();
      vl53l8cx_sub.cancel();
      htpa32_sub.cancel();
      log_sub.cancel();

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}
