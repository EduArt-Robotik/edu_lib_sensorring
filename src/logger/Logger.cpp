#include "sensorring/logger/Logger.hpp"

#include <algorithm>

#include "sensorring/logger/LoggerClient.hpp"

namespace eduart {

namespace logger {

Logger* Logger::getInstance() noexcept {
  static Logger* instance = new Logger;
  return instance;
}

void Logger::registerClient(LoggerClient* client) noexcept {
  if (client) {
    LockGuard lock(_client_mutex);
    auto result = _clients.insert(client);

    // Check if the client was registered
    if (result.second) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Registered new LoggerClient");
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "LoggerClient is already registered");
    }
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "LoggerClient to be registered is not valid");
  }
}

void Logger::unregisterClient(LoggerClient* client) noexcept {
  if (client) {
    LockGuard lock(_client_mutex);
    auto result = _clients.erase(client);

    // Check if the client was removed
    if (result > 0) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Removed measurement client");
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Measurement client to be removed is not registered");
    }
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Measurement client to be removed is not valid");
  }
}

void Logger::log(const LogVerbosity verbosity, const std::string& msg) const {
  LockGuard lock(_client_mutex);
  for (auto& client : _clients) {
    if (client)
      client->onOutputLog(verbosity, msg);
  }
  if (verbosity == LogVerbosity::Exception) {
    throw std::runtime_error(msg);
  }
}

void Logger::log(const LogVerbosity verbosity, const std::stringstream& msg) const {
  log(verbosity, msg.str());
}

} // namespace logger

} // namespace eduart