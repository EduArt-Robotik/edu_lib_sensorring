// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example receives measurements and prints the current measurement rate to the command line.
 * @date 2025-11-18
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sensorring/MeasurementManager.hpp>
#include <thread>

#include "MeasurementProxy.hpp"

using namespace eduart;
using namespace std::chrono_literals;

static constexpr std::string_view INTERFACE_NAME   = "0";
static constexpr com::InterfaceType INTERFACE_TYPE = com::InterfaceType::USBTINGO;

int main(int, char*[]) {
  std::cout << "==========================" << std::endl;
  std::cout << "Minimal sensorring example" << std::endl;
  std::cout << "==========================" << std::endl;
  std::cout << std::endl;

  // Create the parameter structure that is used to instantiate the sensorring
  manager::ManagerParams params;
  {
    sensor::TofSensorParams tof;
    tof.user_idx = 0;
    tof.enable   = true;

    sensor::SensorBoardParams board;
    board.tof_params = tof;

    bus::BusParams bus;
    bus.interface_name = INTERFACE_NAME;
    bus.type           = INTERFACE_TYPE;
    bus.board_param_vec.push_back(board);

    ring::RingParams ring;
    ring.bus_param_vec.push_back(bus);

    params.ring_params = ring;
  }

  // Instantiate a Measurement proxy
  auto proxy = std::make_unique<MeasurementProxy>();

  try {
    // Instantiate a MeasurementManager with the parameters from above
    auto manager = std::make_unique<manager::MeasurementManager>(params);

    // Register the proxy with the LogMeasurementManager to get the measurements
    manager->registerClient(proxy.get());

    // Start the measurements
    manager->startMeasuring();

    while (!proxy->gotFirstMeasurement() && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Printing measurement rate:" << std::endl;
      while (manager->isMeasuring()) {
        std::cout << "Current rate: " << std::fixed << std::setprecision(2) << std::setw(5) << proxy->getRate() << " Hz\r" << std::flush;
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