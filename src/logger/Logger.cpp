#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace logger {

Logger* Logger::getInstance() noexcept {
  // Intentional leak: the singleton is allocated once and never deleted.
  // This avoids the static destruction order fiasco, ensuring the Logger
  // remains available until process exit.
  static Logger* instance = new Logger;
  return instance;
}

subscription::Subscription Logger::subscribe(std::function<void(const LogVerbosity verbosity, const std::string& msg)> callback) {
  return _publisher.subscribe(std::move(callback));
}

void Logger::unsubscribe(subscription::SubscriberToken token) {
  _publisher.unsubscribe(token);
}

void Logger::log(const LogVerbosity verbosity, const std::string& msg) const {
  auto callbacks = _publisher.copySubscribers();

  for (const auto& cb : callbacks) {
    try {
      cb(verbosity, msg);
    } catch (const std::exception& e) {
      // Avoid recursive logging by not calling log() here for Exception verbosity.
      if (verbosity != LogVerbosity::Exception) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Logger subscription callback threw: " + std::string(e.what()));
      }
    } catch (...) {
      if (verbosity != LogVerbosity::Exception) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Logger subscription callback threw unknown exception.");
      }
    }
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