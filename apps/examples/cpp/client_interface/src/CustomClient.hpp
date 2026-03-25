// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   CustomClient.hpp
 * @author EduArt Robotik GmbH
 * @brief  Client class inheriting from the MeasurementClient and LoggerClient interfaces
 * @date 2025-11-18
 */

#pragma once

#include <sensorring/logger/LoggerClient.hpp>
#include <sensorring/manager/MeasurementClient.hpp>

#include "Rate.hpp"

namespace eduart {

/**
 * @class Client class that inherits from the MeasurementClient and LoggerClient
 * interfaces to receive measurements and log output from the sensorring library
 */
class CustomClient : public manager::MeasurementClient, public logger::LoggerClient {
public:
  /// Constructor
  CustomClient(manager::MeasurementManager* manager) noexcept;

  /// Destructor
  ~CustomClient() noexcept override;

  /**
   * @brief Callback method for manager state changes
   * @param state The new state of the manager
   */
  void onStateChange(const manager::ManagerState state) override;

  /**
   * @brief Callback method for raw Time-of-Flight measurements
   * @param measurement_vec The most recent ToF sensor measurements
   */
  void onRawTofMeasurement(const std::vector<measurement::TofMeasurement>& measurement_vec) override;

  /**
   * @brief Callback method for thermal measurements
   * @param measurement_vec The most recent thermal sensor measurements
   */
  void onThermalMeasurement(const std::vector<measurement::ThermalMeasurement>& measurement_vec) override;

  /**
   * @brief Callback method for log output
   * @param verbosity The verbosity level of the log message
   * @param msg The log message
   */
  void onOutputLog(logger::LogVerbosity verbosity, const std::string& msg) override;

public:
  Rate vl53l8cx_rate;
  Rate htpa32_rate;
};

} // namespace eduart