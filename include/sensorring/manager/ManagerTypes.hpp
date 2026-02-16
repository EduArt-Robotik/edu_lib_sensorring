#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

/**
 * @enum DeviceGroup
 * @brief Device types managed by the MeasurementManager.
 */
enum class SENSORRING_EXPORT DeviceGroup {
  VL53L8CX,
  HTPA32,
  WS2812B
};

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