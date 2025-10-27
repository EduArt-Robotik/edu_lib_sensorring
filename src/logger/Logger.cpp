#include "Logger.hpp"

#include <algorithm>
#include <memory>

#include "LoggerClient.hpp"

namespace eduart {

namespace logger {

std::unique_ptr<Logger> Logger::_instance = nullptr;

std::once_flag Logger::_initInstanceFlag;

Logger* Logger::getInstance() {
  std::call_once(_initInstanceFlag, [&] { _instance = std::unique_ptr<Logger>(new Logger()); });
  return _instance.get();
}

void Logger::registerClient(LoggerClient* observer) {
  if (observer && std::find(_observer_vec.begin(), _observer_vec.end(), observer) == _observer_vec.end()) {
    Logger::getInstance()->log(LogVerbosity::Debug, "Registered new observer");
    _observer_vec.push_back(observer);
    return;
  }
  Logger::getInstance()->log(LogVerbosity::Warning, "Observer already registered");
}

void Logger::unregisterClient(LoggerClient* observer) {
  if (observer) {
    const auto& it = std::find(_observer_vec.begin(), _observer_vec.end(), observer);
    if (it != _observer_vec.end()) {
      _observer_vec.erase(it);
    }
  }
}

void Logger::log(const LogVerbosity verbosity, const std::string msg) const {
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