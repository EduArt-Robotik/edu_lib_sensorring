// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   CustomProxy.hpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class to demonstrate object-oriented measurement handling in the sensorring library
 * @date 2025-11-18
 */

#pragma once

#include <functional>
#include <iostream>
#include <sensorring/device/depth/DepthSensor.hpp>
#include <sensorring/device/thermal/ThermalSensor.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <sensorring/subscription/Subscription.hpp>
#include <vector>

#include "Rate.hpp"

namespace eduart {

namespace sensorring {

/**
 * @class Proxy class that implements the sensorring callbacks to get
 * measurements and the log output of the sensorring library
 */
class CustomProxy {
public:
  /// Constructor
  CustomProxy(manager::MeasurementManager* manager) noexcept {
    // Subscribe to the manager state changes
    _subscriptions.emplace_back(manager->subscribeToStateChanges(std::bind(&CustomProxy::onManagerStateChange, this, std::placeholders::_1)));

    // Subscribe to depth and thermal sensors via subscribeAll (synchronized frame delivery)
    _subscriptions.emplace_back(manager->depthSensors().subscribeAll(std::bind(&CustomProxy::onDepthFrame, this, std::placeholders::_1)));
    _subscriptions.emplace_back(manager->thermalSensors().subscribeAll(std::bind(&CustomProxy::onThermalFrame, this, std::placeholders::_1)));

    _depth_sensor_count   = static_cast<unsigned int>(manager->depthSensors().size());
    _thermal_sensor_count = static_cast<unsigned int>(manager->thermalSensors().size());
  }

  /// Destructor
  ~CustomProxy() noexcept {
    // All subscriptions are automatically cancelled when the vector is destroyed
  }

  /**
   * @brief Callback method for manager state changes
   * @param state The new state of the manager
   */
  void onManagerStateChange(const manager::ManagerState state) { std::cout << "[State] State changed to: " << state << std::endl; }

  /**
   * @brief Callback method for a complete depth frame
   * @param measurements Vector of depth measurements (one per sensor)
   */
  void onDepthFrame(const std::vector<measurement::DepthMeasurement>&) { vl53l8cx_rate.tick(_depth_sensor_count); }

  /**
   * @brief Callback method for a complete thermal frame
   * @param measurements Vector of thermal measurements (one per sensor)
   */
  void onThermalFrame(const std::vector<measurement::ThermalMeasurement>&) { htpa32_rate.tick(_thermal_sensor_count); }

public:
  Rate vl53l8cx_rate;
  Rate htpa32_rate;

private:
  std::vector<subscription::Subscription> _subscriptions;
  unsigned int _depth_sensor_count   = 0;
  unsigned int _thermal_sensor_count = 0;
};

} // namespace sensorring

} // namespace eduart