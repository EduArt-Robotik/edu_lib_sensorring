#include "sensorring/logger/LoggerClient.hpp"

#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace logger {

std::string toString(LogVerbosity verbosity) noexcept {
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

std::ostream& operator<<(std::ostream& os, LogVerbosity verbosity) noexcept {
  return os << toString(verbosity);
}

LoggerClient::LoggerClient() noexcept {
  // Register the client with the Logger to get the log output
  _subscription = logger::Logger::getInstance()->subscribe(std::bind(&LoggerClient::onOutputLog, this, std::placeholders::_1, std::placeholders::_2));
}

LoggerClient::~LoggerClient() noexcept {
  // Unregister the client from the Logger to stop receiving log output
  _subscription.cancel();
}

} // namespace logger

} // namespace eduart