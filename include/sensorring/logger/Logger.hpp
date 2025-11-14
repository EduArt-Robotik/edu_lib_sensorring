#pragma once

#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "LoggerClient.hpp"
#include "SensorringExport.hpp"

namespace eduart {

namespace logger {

class SENSORRING_API Logger {
public:
  ~Logger() = default;

  static Logger* getInstance();

  void registerClient(logger::LoggerClient* observer);
  void unregisterClient(logger::LoggerClient* observer);

  void log(const LogVerbosity verbosity, const std::string msg) const;
  void log(const LogVerbosity verbosity, const std::stringstream msg) const;

private:
  Logger() = default;

  mutable std::mutex _client_mutex;
  using LockGuard = std::lock_guard<std::mutex>;

  std::vector<logger::LoggerClient*> _observer_vec;
};

} // namespace logger

} // namespace eduart