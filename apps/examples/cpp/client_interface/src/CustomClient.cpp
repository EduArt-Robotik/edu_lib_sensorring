// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   CustomClient.cpp
 * @author EduArt Robotik GmbH
 * @brief  Client class inheriting from the MeasurementClient and LoggerClient interfaces
 * @date 2025-11-18
 */

#include "CustomClient.hpp"

#include <iostream>

namespace eduart {

CustomClient::CustomClient(manager::MeasurementManager* manager) noexcept {
  registerClient(manager);
}

CustomClient::~CustomClient() noexcept {
  unregisterClient();
}

void CustomClient::onStateChange(const manager::ManagerState state) {
  std::cout << "[State] State changed to: " << state << std::endl;
}

void CustomClient::onRawTofMeasurement(const std::vector<measurement::TofMeasurement>& measurement_vec) {
  vl53l8cx_rate.tick(measurement_vec.size());
}

void CustomClient::onThermalMeasurement(const std::vector<measurement::ThermalMeasurement>& measurement_vec) {
  htpa32_rate.tick(measurement_vec.size());
}

void CustomClient::onOutputLog(logger::LogVerbosity verbosity, const std::string& msg) {
  if (verbosity > logger::LogVerbosity::Debug) {
    std::cout << "[" << verbosity << "] " << msg << std::endl;
  }
}

} // namespace eduart