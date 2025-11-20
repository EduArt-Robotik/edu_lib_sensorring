// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Logger.hpp
 * @author EduArt Robotik GmbH
 * @brief  Logger of the sensorring library
 * @date 2024-11-25
 */

#pragma once

#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "sensorring/logger/LoggerClient.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace logger {

/**
 * @class Logger
 * @brief Centralized class to collect all log messages and relay them to the registered observers. The Logger is implemented as a singleton.
 */
class SENSORRING_API Logger {
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
   * @param[in] observer LoggerClient that will be registered
   */
  void registerClient(LoggerClient* observer) noexcept;

  /**
   * @brief Unregister a new LoggerClient to no longer be notified of log messages
   * @param[in] observer LoggerClient that will be unregistered
   */
  void unregisterClient(LoggerClient* observer) noexcept;

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

  mutable std::mutex _client_mutex;
  using LockGuard = std::lock_guard<std::mutex>;

  std::vector<logger::LoggerClient*> _observer_vec;
};

} // namespace logger

} // namespace eduart