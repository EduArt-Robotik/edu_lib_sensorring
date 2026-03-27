// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   using_client_interface.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example shows how to use the optional MeasurementClient and LoggerClient interfaces.
 * @date 2025-11-18
 */

#include <iomanip>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

#include "CustomClient.hpp"

using namespace eduart;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE = com::InterfaceType::SocketCan;

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::UsbTingo;

int main(int, char*[]) {
  std::cout << "====================================================" << std::endl;
  std::cout << "Minimal sensorring example (using client interface)" << std::endl;
  std::cout << "====================================================" << std::endl;
  std::cout << std::endl;

  manager::ManagerParams params;

  com::ComInterfaceID can_interface;
  can_interface.type = CAN_INTERFACE_TYPE;
  can_interface.name = CAN_INTERFACE_NAME;

  com::ComInterfaceID usbtingo_interface;
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE;
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME;

  try {
    // Create SensorRing via factory auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.addInterface(usbtingo_interface);
    auto sensor_ring = factory.build(ring::ValidationMode::Relaxed);

    if (!sensor_ring) {
      std::cout << "Failed to create SensorRing. Exiting." << std::endl;
      return 1;
    }

    auto manager = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));

    // Instantiate a CustomClient that registers itself with the manager
    auto client = std::make_unique<CustomClient>(manager.get());

    // Start the measurements
    manager->startMeasuring();

    while (!(client->vl53l8cx_rate.gotFirstMeasurement() || client->htpa32_rate.gotFirstMeasurement()) && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Start printing measurement rate." << std::endl;
      while (manager->isMeasuring()) {
        std::cout << "Current measurement rate: " << std::fixed << std::setprecision(2) << std::setw(5) << client->vl53l8cx_rate.getRate() << " Hz (ToF) from " << client->vl53l8cx_rate.getSensorCount() << " sensors, " << std::setw(5)
                  << client->htpa32_rate.getRate() << " Hz (Thermal) from " << client->htpa32_rate.getSensorCount() << " sensors\r" << std::flush;
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
