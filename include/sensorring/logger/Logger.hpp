#pragma once

#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "LoggerClient.hpp"

namespace eduart {

namespace logger {

class Logger {
public:
  ~Logger() = default;

  static Logger* getInstance();

  void registerClient(logger::LoggerClient* observer);
  void unregisterClient(logger::LoggerClient* observer);

  void log(const LogVerbosity verbosity, const std::string msg) const;
  void log(const LogVerbosity verbosity, const std::stringstream msg) const;

private:
  Logger() = default;

  static std::once_flag _initInstanceFlag;
  static std::unique_ptr<Logger> _instance;

  std::vector<logger::LoggerClient*> _observer_vec;
};

}

}