// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   LoggerClient.hpp
 * @author EduArt Robotik GmbH
 * @brief  LoggerClient that can be registered with the Logger to receive log messages
 * @date   2024-11-25
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/logger/LoggerTypes.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/Subscription.hpp"

namespace eduart {

namespace logger {

/**
 * @class LoggerClient
 * @brief Observer interface of the Logger class. Defines the callback method that is triggered by the Logger.
 */
class SENSORRING_EXPORT LoggerClient {
public:
  /// Constructor
  LoggerClient() noexcept;

  /// Destructor
  virtual ~LoggerClient() noexcept;

  /**
   * @brief Callback method for the log output of the sensorring library
   * @param[in] verbosity verbosity level of the log message
   * @param[in] msg       log message string
   */
  virtual void onOutputLog(LogVerbosity verbosity, const std::string& msg) = 0;

private:
  Subscription _subscription;
};

} // namespace logger

} // namespace eduart