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
#include <sensorring/device/DepthSensor.hpp>
#include <sensorring/device/ThermalSensor.hpp>
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

    // Subscribe to depth and thermal sensors via the new typed API
    _subscriptions.emplace_back(manager->depthSensors().subscribe(std::bind(&CustomProxy::onDepthMeasurement, this, std::placeholders::_1)));
    _subscriptions.emplace_back(manager->thermalSensors().subscribe(std::bind(&CustomProxy::onThermalMeasurement, this, std::placeholders::_1)));
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
   * @brief Callback method for depth measurements
   * @param meas The latest depth measurement
   */
  void onDepthMeasurement(const measurement::DepthMeasurement& meas) {
    (void)meas;
    vl53l8cx_rate.tick(1);
  }

  /**
   * @brief Callback method for thermal measurements
   * @param meas The latest thermal measurement
   */
  void onThermalMeasurement(const measurement::ThermalMeasurement& meas) {
    (void)meas;
    htpa32_rate.tick(1);
  }

public:
  Rate vl53l8cx_rate;
  Rate htpa32_rate;

private:
  std::vector<subscription::Subscription> _subscriptions;
};

} // namespace sensorring

} // namespace eduart