// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementClient.hpp
 * @author EduArt Robotik GmbH
 * @brief  MeasurementClient that can be registered with the Logger to receive measurement data
 * @date   2024-12-25
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"
#include "sensorring/types/TofMeasurement.hpp"

namespace eduart {

namespace manager {

/**
 * @enum ManagerState
 * @brief Health state of the sensorring state machine worker
 */
enum class SENSORRING_API ManagerState {
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
SENSORRING_API std::string toString(ManagerState state) noexcept;

/**
 * @brief  Output stream operator for the ManagerState enum class members
 * @param[in] state to be printed as stream
 * @return Stream of the states name written out
 */
SENSORRING_API std::ostream& operator<<(std::ostream& os, ManagerState state) noexcept;

/**
 * @class MeasurementClient
 * @brief Observer interface of the MeasurementManager class. Defines the
 * callback methods that are triggered by the MeasurementManager. It is possible
 * to implement only one or a selection of the callback methods.
 */
class SENSORRING_API MeasurementClient {
public:
  /// Destructor
  virtual ~MeasurementClient() = default;

  /**
   * Callback method for state changes of the state machine worker
   * @param[in] state the new state of the state machine worker
   */
  virtual void onStateChange([[maybe_unused]] const ManagerState state) {};

  /**
   * Callback method for new Time-of-Flight sensor measurements. Returns a
   * vector of the raw measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the individual sensor coordinate frames
   */
  virtual void onRawTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) {};

  /**
   * Callback method for new Time-of-Flight sensor measurements. Returns a
   * vector of the transformed measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the common transformed coordinate frame
   */
  virtual void onTransformedTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) {};

  /**
   * Callback method for new thermal sensor measurements. Returns a
   * vector of the measurements from all sensors.
   * @param[in] measurement the most recent thermal sensor
   * measurements in the common transformed coordinate frame
   */
  virtual void onThermalMeasurement([[maybe_unused]] const std::vector<measurement::ThermalMeasurement>& measurement_vec) {};
};

} // namespace manager

} // namespace eduart