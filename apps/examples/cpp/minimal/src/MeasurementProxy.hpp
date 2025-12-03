// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementProxy.hpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class for the minimal C++ example
 * @date 2025-11-18
 */

#pragma once

#include <atomic>
#include <chrono>
#include <sensorring/MeasurementClient.hpp>
#include <sensorring/logger/LoggerClient.hpp>

namespace eduart {

/**
 * @class Proxy class that implements the sensorring callbacks to get
 * measurements and the log output of the sensorring library
 */
class MeasurementProxy : public manager::MeasurementClient, public logger::LoggerClient {
public:
  /**
   * @brief Get the rate of measurements since the last call of this method
   * @return measurement rate in Hz
   */
  double getRate();

  /**
   * @brief Check if the client got the first measurement
   * @return true if the client already got the first measurement
   */
  bool gotFirstMeasurement();

  /**
   * @brief Implement onRawTofMeasurement callback method
   */
  void onRawTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) override;

  /**
   * @brief Implement onRawTofMeasurement callback method
   */
  void onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] const std::string& msg) override;

private:
  using Clock     = std::chrono::steady_clock;
  using Duration  = Clock::duration;
  using TimePoint = Clock::time_point;
  using toSeconds = std::chrono::duration<double>;

  Duration _duration                 = Duration::zero();
  TimePoint _last_measurement        = Clock::now();
  std::atomic<bool> _init_flag       = false;
  std::atomic<unsigned int> _counter = 0;
};

} // namespace eduart