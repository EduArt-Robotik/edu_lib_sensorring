// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Logger.hpp
 * @author EduArt Robotik GmbH
 * @brief  Logger of the sensorring library
 * @date   2024-11-25
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "sensorring/logger/LoggerClient.hpp"
#include "sensorring/types/SubscriberToken.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace logger {

/**
 * @class Logger
 * @brief Centralized class to collect all log messages and relay them to the registered observers. The Logger is implemented as a singleton.
 */
class SENSORRING_EXPORT Logger {
public:
  /// Destructor
  ~Logger() = default;

  /**
   * @brief Get a reference to the instance of the Logger singleton
   * @return Pointer to the Logger instance
   */
  static Logger* getInstance() noexcept;

  /**
   * @brief Register a new LoggerClient to be notified of future log messages
   * @param[in] client LoggerClient that will be registered
   */
  void registerClient(LoggerClient* client) noexcept;

  /**
   * @brief Unregister a new LoggerClient to no longer be notified of log messages
   * @param[in] client LoggerClient that will be unregistered
   */
  void unregisterClient(LoggerClient* client) noexcept;

  /**
   * @brief Subscribe to log messages
   * @param[in] token Subscription token
   * @param[in] callback Callback function to be called when a log message is received
   */
   SubscriberToken subscribe(std::function<void(const LogVerbosity verbosity, const std::string& msg)> callback);

  /**
   * @brief Unsubscribe from log messages
   * @param[in] token Subscription token
   */
  void unsubscribe(SubscriberToken token);

  /**
   * @brief Log a message that will be relayed to all registered observers
   * @param[in] verbosity Log verbosity of the message
   * @param[in] msg Log message
   * @throw Throws std::runtime_error when a message with LogVerbosity::Exception is logged
   */
  void log(const LogVerbosity verbosity, const std::string& msg) const;

  /**
   * @brief Log a message that will be relayed to all registered observers
   * @param[in] verbosity Log verbosity of the message
   * @param[in] msg Log message
   * @throw Throws std::runtime_error when a message with LogVerbosity::Exception is logged
   */
  void log(const LogVerbosity verbosity, const std::stringstream& msg) const;

private:
  /// Private constructor. The Logger is a singleton.
  Logger() = default;

  mutable std::recursive_mutex _client_mutex;
  using LockGuard = std::lock_guard<std::recursive_mutex>;

  std::set<logger::LoggerClient*> _clients;

  mutable std::recursive_mutex _subscriber_mutex;
  std::unordered_map<SubscriberToken, std::function<void(const LogVerbosity verbosity, const std::string& msg)> > _subscriptions;
};

} // namespace logger

} // namespace eduart