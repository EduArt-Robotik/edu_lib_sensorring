#pragma once

#include <string>
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace logger {

/**
 * @enum LogVerbosity
 * @brief Verbosity levels of the logger output
 * @author Hannes Duske
 * @date 25.12.2024
 */
enum class SENSORRING_API LogVerbosity {
  Debug,
  Info,
  Warning,
  Error,
  Exception
};

/**
 * @class LoggerClient
 * @brief Observer interface of the Logger class. Defines the callback method
 * that is triggered by the Logger.
 * @author Hannes Duske
 * @date 25.12.2024
 */
class SENSORRING_API LoggerClient {
public:
  /**
   * Callback method for the log output of the sensorring library
   * @param[in] verbosity verbosity level of the log message
   * @param[in] msg       log message string
   */
  virtual void onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] std::string msg) {};
};

} // namespace logger

} // namespace eduart