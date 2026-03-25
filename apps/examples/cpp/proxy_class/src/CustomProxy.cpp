// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   CustomProxy.cpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class to demonstrate object-oriented measurement handling in the sensorring library
 * @date 2025-11-18
 */

#include "CustomProxy.hpp"

#include <functional>
#include <iostream>
#include <sensorring/logger/Logger.hpp>

namespace eduart {

CustomProxy::CustomProxy(manager::MeasurementManager* manager) noexcept : _manager(manager) {

  _logger_subscription = logger::Logger::getInstance()->subscribe(std::bind(&CustomProxy::onLogOutput, this, std::placeholders::_1, std::placeholders::_2));

  // Subscribe to the manager state changes and measurements
  _manager_subscriptions.emplace_back(manager->subscribeToStateChanges(std::bind(&CustomProxy::onManagerStateChange, this, std::placeholders::_1)));
  _manager_subscriptions.emplace_back(manager->subscribeToDeviceGroup(device::DeviceType::VL53L8CX, std::bind(&CustomProxy::onVL53L8CXCallback, this, std::placeholders::_1)));
  _manager_subscriptions.emplace_back(manager->subscribeToDeviceGroup(device::DeviceType::HTPA32, std::bind(&CustomProxy::onHTPA32Callback, this, std::placeholders::_1)));
  _manager_subscriptions.emplace_back(manager->subscribeToDeviceGroup(device::DeviceType::WS2812b, std::bind(&CustomProxy::onWS2812bCallback, this, std::placeholders::_1)));
}

CustomProxy::~CustomProxy() noexcept {

  logger::Logger::getInstance()->unsubscribe(_logger_subscription);

  for (const auto& sub : _manager_subscriptions) {
    _manager->unsubscribe(sub);
  }
}

void CustomProxy::onLogOutput(logger::LogVerbosity verbosity, const std::string& msg) {
  if (verbosity > logger::LogVerbosity::Debug) {
    std::cout << "[" << verbosity << "] " << msg << std::endl;
  }
}

void CustomProxy::onManagerStateChange(const manager::ManagerState state) {
  std::cout << "[State] State changed to: " << state << std::endl;
}

void CustomProxy::onVL53L8CXCallback(const device::DeviceGroup& group) {
  vl53l8cx_rate.tick(group.getDeviceCount());
}

void CustomProxy::onHTPA32Callback(const device::DeviceGroup& group) {
  htpa32_rate.tick(group.getDeviceCount());
}

void CustomProxy::onWS2812bCallback(const device::DeviceGroup& group) {
  (void)group;
}

} // namespace eduart