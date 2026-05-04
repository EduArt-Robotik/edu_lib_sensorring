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
#include <sensorring/device/DepthSensor.hpp>
#include <sensorring/device/ThermalSensor.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

#include "Rate.hpp"

using namespace eduart::sensorring;
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

    // Create the MeasurementManager directly from the factory
    auto manager = std::make_unique<manager::MeasurementManager>(params, factory);

    const auto depth_sensor_count   = static_cast<unsigned int>(manager->depthSensors().size());
    const auto thermal_sensor_count = static_cast<unsigned int>(manager->thermalSensors().size());

    // Subscribe to the state changes to get the measurements
    auto state_sub = manager->subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "[State] State changed to: " << state << std::endl;
    });

    // Subscribe to all depth sensors for measurement rate tracking
    auto depth_sub = manager->depthSensors().subscribe([&vl53l8cx_rate, depth_sensor_count](const measurement::DepthMeasurement& meas) {
      if (meas.sensor_index == 0) {
        vl53l8cx_rate->tick(depth_sensor_count);
      }
    });

    // Subscribe to all thermal sensors for measurement rate tracking
    auto thermal_sub = manager->thermalSensors().subscribe([&htpa32_rate, thermal_sensor_count](const measurement::ThermalMeasurement& meas) {
      if (meas.sensor_index == 0) {
        htpa32_rate->tick(thermal_sensor_count);
      }
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
      depth_sub.cancel();
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
