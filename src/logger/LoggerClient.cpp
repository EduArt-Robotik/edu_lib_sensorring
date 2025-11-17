#include "sensorring/logger/LoggerClient.hpp"

namespace eduart {

namespace logger {

std::string to_string(LogVerbosity verbosity) {
  switch (verbosity) {
  case LogVerbosity::Debug:
    return "Debug";
  case LogVerbosity::Info:
    return "Info";
  case LogVerbosity::Warning:
    return "Warning";
  case LogVerbosity::Error:
    return "Error";
  case LogVerbosity::Exception:
    return "Exception";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, LogVerbosity verbosity) {
  return os << to_string(verbosity);
}

} // namespace logger

} // namespace eduart