// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example receives measurements and prints the current measurement rate to the command line.
 * @date 2025-11-18
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/logger/Logger.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

using namespace eduart;
using namespace std::chrono_literals;

using Clock     = std::chrono::steady_clock;
using Duration  = Clock::duration;
using TimePoint = Clock::time_point;
using toSeconds = std::chrono::duration<double>;

// Default CAN interface (expects a SocketCAN interface named "can0" to be available)
static constexpr std::string_view CAN_INTERFACE_NAME        = "can0";
static constexpr com::InterfaceType CAN_INTERFACE_TYPE      = com::InterfaceType::SOCKETCAN;

// Default USBtingo interface (uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME   = "0";
static constexpr com::InterfaceType USBTINGO_INTERFACE_TYPE = com::InterfaceType::USBTINGO;

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
  params.frequency_thermal_hz = 5.0;

  auto vl53l8cx_rate = std::make_unique<Rate>();
  auto htpa32_rate   = std::make_unique<Rate>();

  com::ComInterfaceID can_interface;
  can_interface.type = CAN_INTERFACE_TYPE;
  can_interface.name = CAN_INTERFACE_NAME;

  com::ComInterfaceID usbtingo_interface;
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE;
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME;


  try {
    // Subscribe to the log messages
    auto log_sub = logger::Logger::getInstance()->subscribe([](const logger::LogVerbosity verbosity, const std::string& msg) {
      // if (verbosity > logger::LogVerbosity::Debug)
      std::cout << "[" << verbosity << "] " << msg << std::endl;
    });

    // Create the SensorRing via auto-discovery
    ring::SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.addInterface(usbtingo_interface);
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

    // Subscribe to the ToF device group to get the measurements
    std::atomic<unsigned int> vl53l8cx_sensor_count = 0;
    auto vl53l8cx_sub = manager->subscribeToDeviceGroup(device::DeviceType::VL53L8CX, [&vl53l8cx_rate, &vl53l8cx_sensor_count](const device::DeviceGroup& group) {
      vl53l8cx_rate->tick();
      vl53l8cx_sensor_count = group.getDeviceCount();
    });

    // Subscribe to the Thermal device group to get the measurements
    std::atomic<unsigned int> htpa32_sensor_count = 0;
    auto htpa32_sub = manager->subscribeToDeviceGroup(device::DeviceType::HTPA32, [&htpa32_rate, &htpa32_sensor_count](const device::DeviceGroup& group) {
      htpa32_rate->tick();
      htpa32_sensor_count = group.getDeviceCount();
    });

    // Start the measurements
    manager->startMeasuring();

    while (!vl53l8cx_rate->gotFirstMeasurement() && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Start printing measurement rate." << std::endl;
      while (manager->isMeasuring()) {
        std::cout << "Current measurement rate: " << std::fixed << std::setprecision(2) << std::setw(5) << vl53l8cx_rate->getRate() << " Hz (ToF) from " << vl53l8cx_sensor_count << " sensors, " << std::setw(5) << htpa32_rate->getRate() << " Hz (Thermal) from " << htpa32_sensor_count << " sensors\r" << std::flush;
        std::this_thread::sleep_for(1s);
      }

      // Unsubscribe from manager before stopping (optional)
      manager->unsubscribe(state_sub);
      manager->unsubscribe(vl53l8cx_sub);
      manager->unsubscribe(htpa32_sub);

      // Unsubscribe from logger before stopping (optional)
      logger::Logger::getInstance()->unsubscribe(log_sub);

      // Stop the measurements
      manager->stopMeasuring();
    }

  } catch (const std::exception& e) {
    std::cout << "Caught exception in example application: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}