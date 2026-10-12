// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DeviceState.hpp
 * @author EduArt Robotik GmbH
 * @brief  Unified device state enum covering lifecycle and runtime health.
 * @date   2025-02-09
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @enum DeviceState
 * @brief Unified lifecycle and runtime state of a device.
 *
 * Covers both the device lifecycle (Undefined → Initialized → Idle → Shutdown)
 * and runtime health (Ok, ReceiveError, ProcessError, Error).
 */
enum class DeviceState {
  Undefined,    ///< Initial state; device not yet configured.
  Initialized,  ///< Device has been configured but not yet started.
  Idle,         ///< Device is running but not actively measuring.
  Ok,           ///< Device is measuring and data is valid.
  ReceiveError, ///< A CAN receive error was detected.
  ProcessError, ///< An error occurred while processing received data.
  Error,        ///< General unrecoverable device error.
  Shutdown      ///< Device has been shut down and should not be used.
};

/**
 * @brief Convert a DeviceState value to its string representation.
 * @param[in] state The state to convert.
 * @return Name of the state as a string.
 */
SENSORRING_EXPORT std::string toString(DeviceState state) noexcept;

/**
 * @brief Output stream operator for DeviceState.
 * @param[in] os Output stream to write to.
 * @param[in] state The state to print.
 * @return The stream with the state name written out.
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, DeviceState state) noexcept;

} // namespace device

} // namespace sensorring

} // namespace eduart
