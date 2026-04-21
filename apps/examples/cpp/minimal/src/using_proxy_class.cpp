// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   using_proxy_class.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example shows how to use the SensorRing with a proxy class for object-oriented measurement handling.
 * @date 2025-11-18
 */

#include <iomanip>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

#include "CustomProxy.hpp"

using namespace eduart::sensorring;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SocketCan;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::UsbTingo;

int main(int, char*[]) {
  std::cout << "===============================================" << std::endl;
  std::cout << "Minimal sensorring example (using proxy class)" << std::endl;
  std::cout << "===============================================" << std::endl;
  std::cout << std::endl;

  manager::ManagerParams params;

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

    // Create SensorRing via factory auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.addInterface(usbtingo_interface);

    auto manager = std::make_unique<manager::MeasurementManager>(params, factory);

    // Instantiate a Measurement proxy
    auto proxy = std::make_unique<CustomProxy>(manager.get());

    // Start the measurements
    manager->startMeasuring();

    while (!(proxy->vl53l8cx_rate.gotFirstMeasurement() || proxy->htpa32_rate.gotFirstMeasurement()) && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Start printing measurement rate." << std::endl;
      while (manager->isMeasuring()) {
        std::cout << "Current measurement rate: " << std::fixed << std::setprecision(2) << std::setw(5) << proxy->vl53l8cx_rate.getRate() << " Hz (ToF) from " << proxy->vl53l8cx_rate.getSensorCount() << " sensors, " << std::setw(5)
                  << proxy->htpa32_rate.getRate() << " Hz (Thermal) from " << proxy->htpa32_rate.getSensorCount() << " sensors\r" << std::flush;
        std::this_thread::sleep_for(1s);
      }

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}
