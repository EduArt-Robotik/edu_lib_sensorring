#pragma once

#include <string>

#include "CustomTypes.hpp"

namespace eduart {

namespace manager {

/**
 * @enum ManagerState
 * @brief Health state of the sensorring state machine worker
 * @author Hannes Duske
 * @date 25.12.2024
 */
enum class ManagerState {
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
std::string to_string(ManagerState state);

/**
 * @brief  Output stream operator for the ManagerState enum class members
 * @param[in] state to be printed as stream
 * @return Stream of the states name written out
 */
std::ostream& operator<<(std::ostream& os, ManagerState state);

/**
 * @class MeasurementClient
 * @brief Observer interface of the MeasurementManager class. Defines the
 * callback methods that are triggered by the MeasurementManager. It is possible
 * to implement only one or a selection of the callback methods.
 * @author Hannes Duske
 * @date 25.12.2024
 */
class MeasurementClient {
public:
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
  virtual void onRawTofMeasurement([[maybe_unused]] std::vector<measurement::TofMeasurement> measurement_vec) {};

  /**
   * Callback method for new Time-of-Flight sensor measurements.  Returns a
   * vector of the transformed measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the common transformed coordinate frame
   */
  virtual void onTransformedTofMeasurement([[maybe_unused]] std::vector<measurement::TofMeasurement> measurement_vec) {};

  /**
   * Callback method for new thermal sensor measurements. Returns one individual
   * measurements of each thermal sensor.
   * @param[in] idx the index of the thermal sensor that that recorded the
   * measurement. Index starts at zero (0) for the first thermal sensor and only
   * counts active thermal sensors.
   * @param[in] measurement the most recent thermal sensor measurement of the
   * specified sensor
   */
  virtual void onThermalMeasurement([[maybe_unused]] std::vector<measurement::ThermalMeasurement> measurement_vec) {};
};

} // namespace manager

} // namespace eduart