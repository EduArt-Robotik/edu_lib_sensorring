#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "LoggerClient.hpp"
#include "SingletonTemplate.hpp"

namespace eduart {

namespace logger {

class Logger : public Singleton<Logger> {
public:
  ~Logger () = default;

  void registerClient (logger::LoggerClient *observer);
  void unregisterClient (logger::LoggerClient *observer);

  void log (const LogVerbosity verbosity, const std::string msg);
  void log (const LogVerbosity verbosity, const std::stringstream msg);

private:
  friend class Singleton<Logger>;
  Logger () = default;

  std::vector<logger::LoggerClient *> _observer_vec;
};

}

}