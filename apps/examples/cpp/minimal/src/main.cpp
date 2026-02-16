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
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

using namespace eduart;
using namespace std::chrono_literals;

using Clock     = std::chrono::steady_clock;
using Duration  = Clock::duration;
using TimePoint = Clock::time_point;
using toSeconds = std::chrono::duration<double>;

static constexpr std::string_view INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType INTERFACE_TYPE = com::InterfaceType::SOCKETCAN;

struct Rate {
  std::mutex mutex;
  bool init_flag             = false;
  unsigned int counter       = 0;
  Duration duration          = Duration::zero();
  TimePoint last_measurement = Clock::time_point::min();

  void tick() {
    std::lock_guard<std::mutex> lock(mutex);
    init_flag = true;
    duration += Clock::now() - last_measurement;
    last_measurement = Clock::now();
    counter++;
  }

  double getRate() {
    std::lock_guard<std::mutex> lock(mutex);
    if (init_flag) {
      auto rate = static_cast<double>(counter) / toSeconds(duration).count();
      duration  = Duration::zero();
      counter   = 0;
      return rate;
    }
    return 0.0;
  }

  bool gotFirstMeasurement() {
    std::lock_guard<std::mutex> lock(mutex);
    return init_flag;
  }
};

int main(int, char*[]) {
  std::cout << "==========================" << std::endl;
  std::cout << "Minimal sensorring example" << std::endl;
  std::cout << "==========================" << std::endl;
  std::cout << std::endl;

  // Create the parameter structure that is used to instantiate the sensorring
  manager::ManagerParams params;
  ring::RingParams ring;
  {
    device::VL53L8CX_Params tof;
    tof.user_idx = 0;
    tof.enable   = true;

    device::SensorBoardParams board;
    board.vl53l8cx_params = tof;

    bus::BusParams bus;
    bus.interface_name = INTERFACE_NAME;
    bus.type           = INTERFACE_TYPE;
    bus.board_param_vec.push_back(board);

    ring.bus_param_vec.push_back(bus);
  }

  // Instantiate a Measurement proxy
  auto rate = std::make_unique<Rate>();

  try {
    // Create SensorRing from ring params, then instantiate MeasurementManager
    auto sensor_ring = ring::SensorRing::create(ring);
    auto manager     = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));

    // Subscribe to the state changes to get the measurements
    auto state_sub = manager->subscribeToStateChanges([](const manager::ManagerState state) {
      std::cout << "State changed to: " << state << std::endl;
    });

    // Subscribe to the ToF device group to get the measurements
    auto tof_sub = manager->subscribeToDeviceGroup(manager::DeviceGroupKey::ToF, [&rate](const device::DeviceGroup&) {
      rate->tick();
    });

    // Subscribe to the log messages
    auto log_sub = logger::Logger::getInstance()->subscribe([](const logger::LogVerbosity verbosity, const std::string& msg) {
      std::cout << "[" << verbosity << "] " << msg << std::endl;
    });

    // Start the measurements
    manager->startMeasuring();

    while (!rate->gotFirstMeasurement() && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Start printing measurement rate." << std::endl;
      unsigned int counter = 0;
      while (manager->isMeasuring() && counter < 10) {
        std::cout << "Current measurement rate: " << std::fixed << std::setprecision(2) << std::setw(5) << rate->getRate() << " Hz\r" << std::flush;
        std::this_thread::sleep_for(1s);
        counter++;
      }

      // Unsubscribe from manager before stopping (optional)
      manager->unsubscribe(state_sub);
      manager->unsubscribe(tof_sub);

      // Unsubscribe from logger before stopping (optional)
      logger::Logger::getInstance()->unsubscribe(log_sub);

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}