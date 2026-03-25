// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   CustomProxy.hpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class to demonstrate object-oriented measurement handling in the sensorring library
 * @date 2025-11-18
 */

#pragma once

#include <string>
#include <vector>

#include "sensorring/logger/LoggerTypes.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/types/Subscription.hpp"

#include "Rate.hpp"

namespace eduart {

/**
 * @class Proxy class that implements the sensorring callbacks to get
 * measurements and the log output of the sensorring library
 */
class CustomProxy {
public:
  /// Constructor
  CustomProxy(manager::MeasurementManager* manager) noexcept;

  /// Destructor
  ~CustomProxy() noexcept;

  /**
   * @brief Callback method for manager state changes
   * @param state The new state of the manager
   */
  void onManagerStateChange(const manager::ManagerState state);

  /**
   * @brief Callback method for VL53L8CX measurements
   * @param group The device group of active VL53L8CX devices
   */
  void onVL53L8CXCallback(const device::DeviceGroup& group);

  /**
   * @brief Callback method for HTPA32 measurements
   * @param group The device group of active HTPA32 devices
   */
  void onHTPA32Callback(const device::DeviceGroup& group);

  /**
   * @brief Callback method for WS2812b measurements
   * @param group The device group of active WS2812b devices
   */
  void onWS2812bCallback(const device::DeviceGroup& group);

  /**
   * @brief Callback method for log output
   * @param verbosity The verbosity level of the log message
   * @param msg The log message
   */
  void onLogOutput(logger::LogVerbosity verbosity, const std::string& msg);

public:
  Rate vl53l8cx_rate;
  Rate htpa32_rate;

private:
  std::vector<Subscription> _subscriptions;
};

} // namespace eduart