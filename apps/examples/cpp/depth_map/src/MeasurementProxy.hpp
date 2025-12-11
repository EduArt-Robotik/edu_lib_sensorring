// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementProxy.hpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class for the depth map C++ example
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
  /// Constructor
  MeasurementProxy();

  /// Destructor
  ~MeasurementProxy();

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
  static constexpr double MIN_DIST = 0.0;
  static constexpr double MAX_DIST = 1.0;

  std::string depthToColor(double depth, double min, double max);
  void printDepthMap(const measurement::PointCloud& points);

  std::atomic<bool> _init_flag    = false;
  std::atomic<bool> _reset_cursor = false;
};

} // namespace eduart