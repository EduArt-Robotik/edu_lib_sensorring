// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   LogVerbosity.hpp
 * @author EduArt Robotik GmbH
 * @brief  Log verbosity levels for the logger.
 * @date   2024-11-25
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace logger {

/**
 * @enum LogVerbosity
 * @brief Verbosity levels for logger output and filtering.
 */
enum class LogVerbosity {
  /// Fine-grained diagnostic messages.
  Debug,
  /// General informational messages.
  Info,
  /// Warnings that do not stop execution.
  Warning,
  /// Error conditions.
  Error,
  /// Exceptional failures; logging at this level may throw.
  Exception
};

/**
 * @brief Function to convert the LogVerbosity enum class members to string
 * @param[in] verbosity to be converted to a string
 * @return Name of the verbosity level written out as string
 */
SENSORRING_EXPORT std::string toString(LogVerbosity verbosity) noexcept;

/**
 * @brief  Output stream operator for the LogVerbosity enum class members
 * @param[in] os output stream to write to
 * @param[in] verbosity to be printed as stream
 * @return Stream with the verbosity name written out
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, LogVerbosity verbosity) noexcept;

} // namespace logger

} // namespace sensorring

} // namespace eduart