// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   ManagerState.hpp
 * @author EduArt Robotik GmbH
 * @brief  ManagerState and helpers for the MeasurementManager.
 * @date   2024-12-25
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

/**
 * @enum ManagerState
 * @brief Health state of the sensorring state machine worker
 */
enum class SENSORRING_EXPORT ManagerState {
  Uninitialized,
  Initialized,
  Running,
  Shutdown,
  Error
};

/**
 * @brief Function to convert the ManagerState enum class members to string
 * @param[in] state to be converted to a string
 * @return Name of the state written out as string
 */
SENSORRING_EXPORT std::string toString(ManagerState state) noexcept;

/**
 * @brief  Output stream operator for the ManagerState enum class members
 * @param[in] state to be printed as stream
 * @return Stream of the states name written out
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, ManagerState state) noexcept;

} // namespace manager

} // namespace eduart