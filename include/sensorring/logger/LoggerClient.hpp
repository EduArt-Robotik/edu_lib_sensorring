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
enum class SENSORRING_API LogVerbosity {
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
SENSORRING_API std::string toString(LogVerbosity verbosity) noexcept;

/**
 * @brief  Output stream operator for the LogVerbosity enum class members
 * @param[in] verbosity to be printed as stream
 * @return Stream of the states name written out
 */
SENSORRING_API std::ostream& operator<<(std::ostream& os, LogVerbosity verbosity) noexcept;

/**
 * @class LoggerClient
 * @brief Observer interface of the Logger class. Defines the callback method that is triggered by the Logger.
 */
class SENSORRING_API LoggerClient {
public:
  /// Destructor
  virtual ~LoggerClient() = default;

  /**
   * Callback method for the log output of the sensorring library
   * @param[in] verbosity verbosity level of the log message
   * @param[in] msg       log message string
   */
  virtual void onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] std::string msg) {};
};

} // namespace logger

} // namespace eduart