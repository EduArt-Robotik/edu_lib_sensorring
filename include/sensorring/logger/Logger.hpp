// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Logger.hpp
 * @author EduArt Robotik GmbH
 * @brief  Logger of the sensorring library
 * @date   2024-11-25
 */

#pragma once

#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include "sensorring/logger/LoggerTypes.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/Subscription.hpp"
#include "sensorring/types/SubscriberToken.hpp"

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
   * @brief Get a pointer to the instance of the Logger singleton
   * @return Pointer to the Logger instance
   */
  static Logger* getInstance() noexcept;

  /**
   * @brief Subscribe to log messages
   * @param[in] callback Callback function to be called when a log message is received
   * @return RAII Subscription that auto-cancels on destruction
   */
  Subscription subscribe(std::function<void(const LogVerbosity verbosity, const std::string& msg)> callback);

  /**
   * @brief Unsubscribe from log messages
   * @param[in] token Token returned by subscribe
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

  using Mutex     = std::recursive_mutex;
  using LockGuard = std::lock_guard<Mutex>;

  mutable Mutex _subscriber_mutex;
  std::unordered_map<SubscriberToken, std::function<void(const LogVerbosity verbosity, const std::string& msg)> > _subscriptions;
};

} // namespace logger

} // namespace eduart