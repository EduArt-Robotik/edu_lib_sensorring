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
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

#include "MeasurementProxy.hpp"

using namespace eduart;
using namespace std::chrono_literals;

static constexpr std::string_view INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType INTERFACE_TYPE = com::InterfaceType::SOCKETCAN;

int main(int, char*[]) {

  std::cout << "\33c";
  std::cout << "============================" << std::endl;
  std::cout << "Depth map sensorring example" << std::endl;
  std::cout << "============================" << std::endl;
  std::cout << std::endl;

  manager::ManagerParams params;

  com::ComInterfaceID interface;
  interface.type = INTERFACE_TYPE;
  interface.name = INTERFACE_NAME;

  // Instantiate a Measurement proxy
  auto proxy = std::make_unique<MeasurementProxy>();

  try {
    // Create SensorRing via factory auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(interface);
    factory.expectBoard({}, { device::VL53L8CX_Params{} });
    auto sensor_ring = factory.build(ring::ValidationMode::Relaxed);

    if (!sensor_ring) {
      std::cout << "Failed to create SensorRing. Exiting." << std::endl;
      return 1;
    }

    auto manager = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));

    // Register the proxy with the MeasurementManager to get the measurements
    proxy->registerClient(manager.get());

    // Start the measurements
    manager->startMeasuring();

    while (!proxy->gotFirstMeasurement() && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {

      while (manager->isMeasuring()) {
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