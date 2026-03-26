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

Subscription Logger::subscribe(std::function<void(const LogVerbosity verbosity, const std::string& msg)> callback) {
  if (!callback) {
    return Subscription();
  }

  auto token = subscription::SubscriberToken::getNextToken();
  LockGuard lock(_subscriber_mutex);
  _subscriptions.emplace(token, std::move(callback));
  return Subscription(token, [this, token]() {
    unsubscribe(token);
  });
}

void Logger::unsubscribe(subscription::SubscriberToken token) {
  LockGuard lock(_subscriber_mutex);
  _subscriptions.erase(token);
}

void Logger::log(const LogVerbosity verbosity, const std::string& msg) const {
  // Copy the subscriber list under lock, then release before invoking callbacks.
  // This avoids deadlocks if a callback tries to subscribe/unsubscribe.
  std::vector<std::function<void(const LogVerbosity, const std::string&)>> callbacks;
  {
    LockGuard sub_lock(_subscriber_mutex);
    callbacks.reserve(_subscriptions.size());
    for (const auto& sub : _subscriptions) {
      if (sub.second) {
        callbacks.push_back(sub.second);
      }
    }
  }

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