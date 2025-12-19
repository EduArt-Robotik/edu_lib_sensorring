// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   LoggerClient.hpp
 * @author EduArt Robotik GmbH
 * @brief  LoggerClient that can be registered with the Logger to receive log messages
 * @date   2024-11-25
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace logger {

/**
 * @enum LogVerbosity
 * @brief Verbosity levels of the logger output
 */
enum class SENSORRING_EXPORT LogVerbosity {
  Debug,
  Info,
  Warning,
  Error,
  Exception
};

/**
 * @brief Function to convert the LogVerbosity enum class members to string
 * @param[in] verbosity to be converted to a string
 * @return Name of the state written out as string
 */
SENSORRING_EXPORT std::string toString(LogVerbosity verbosity) noexcept;

/**
 * @brief  Output stream operator for the LogVerbosity enum class members
 * @param[in] verbosity to be printed as stream
 * @return Stream of the states name written out
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, LogVerbosity verbosity) noexcept;

/**
 * @class LoggerClient
 * @brief Observer interface of the Logger class. Defines the callback method that is triggered by the Logger.
 */
class SENSORRING_EXPORT LoggerClient {
public:
  /// Destructor
  virtual ~LoggerClient() = default;

  /**
   * Callback method for the log output of the sensorring library
   * @param[in] verbosity verbosity level of the log message
   * @param[in] msg       log message string
   */
  virtual void onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] const std::string& msg) {};
};

} // namespace logger

} // namespace eduart