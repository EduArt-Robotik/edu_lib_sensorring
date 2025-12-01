#include "sensorring/logger/LoggerClient.hpp"

namespace eduart {

namespace logger {

std::string toString(LogVerbosity verbosity) noexcept{
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

std::ostream& operator<<(std::ostream& os, LogVerbosity verbosity) noexcept{
  return os << toString(verbosity);
}

} // namespace logger

} // namespace eduart