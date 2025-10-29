#include "Logger.hpp"

#include <algorithm>

#include "LoggerClient.hpp"

namespace eduart {

namespace logger {

Logger* Logger::getInstance() {
  static Logger* instance = new Logger;
  return instance;
}

void Logger::registerClient(LoggerClient* observer) {
  bool alreadyRegistered = false;

  {
    LockGuard lock(_client_mutex);
    if (observer && std::find(_observer_vec.begin(), _observer_vec.end(), observer) == _observer_vec.end()) {
      _observer_vec.push_back(observer);
    } else {
      alreadyRegistered = true;
    }
  }

  Logger::getInstance()->log(alreadyRegistered ? LogVerbosity::Warning : LogVerbosity::Debug, alreadyRegistered ? "Observer already registered" : "Registered new observer");
}

void Logger::unregisterClient(LoggerClient* observer) {
  LockGuard lock(_client_mutex);
  if (observer) {
    const auto& it = std::find(_observer_vec.begin(), _observer_vec.end(), observer);
    if (it != _observer_vec.end()) {
      _observer_vec.erase(it);
    }
  }
}

void Logger::log(const LogVerbosity verbosity, const std::string msg) const {
  LockGuard lock(_client_mutex);
  for (auto& observer : _observer_vec) {
    if (observer)
      observer->onOutputLog(verbosity, msg);
  }
  if (verbosity == LogVerbosity::Exception) {
    throw std::runtime_error(msg);
  }
}

void Logger::log(const LogVerbosity verbosity, const std::stringstream msg) const {
  log(verbosity, msg.str());
}

} // namespace logger

} // namespace eduart