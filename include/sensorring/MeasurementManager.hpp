#pragma once

#include <memory>
#include <string>

#include "CustomTypes.hpp"
#include "MeasurementClient.hpp"
#include "Parameters.hpp"

namespace eduart {

namespace ring {
class SensorRing;
}

namespace manager {

// Forward declaration of implementation class
class MeasurementManagerImpl;

// Forward declaration
enum class MeasurementState;

/**
 * @class MeasurementManager
 * @brief Meta class that handles the timing, triggering and processing of sensor measurements. Internally it runs a looping state machine.
 * @author Hannes Duske
 * @date 25.12.2024
 */
class MeasurementManager {
public:
  /**
   * Destructor
   */
  ~MeasurementManager();

  /**
   * Run one processing cycle of the state machine worker
   * @return error code
   */
  bool measureSome();

  /**
   * Start running the state machine worker loop in a thread
   * @return error code
   */
  bool startMeasuring();

  /**
   * Stop the state machine worker loop and close the thread
   * @return error code
   */
  bool stopMeasuring();

  /**
   * Register an observer with the MeasurementManager object
   * @param[in] observer Observer that is registered and gets notified on future events
   */
  void registerClient(MeasurementClient* observer);

  /**
   * Unregister an observer with the MeasurementManager object
   * @param[in] observer Observer that is unregistered and will not be notified on future events
   */
  void unregisterClient(MeasurementClient* observer);

  /**
   * Get a string representation of the topology of the connected sensors
   * @return Formatted string describing the topology
   */
  std::string printTopology() const;

  /**
   * Get the health status of the state machine
   * @return Current worker state
   */
  WorkerState getWorkerState() const;

  /**
   * Get the parameters with which the MeasurementManager was initialized
   * @return Initial parameter struct
   */
  ManagerParams getParams() const;

  /**
   * Enable or disable the Time-of-Flight sensor measurements
   * @param[in] state enable signal
   */
  void enableTofMeasurement(bool state);

  /**
   * Enable or disable the thermal sensor measurements
   * @param[in] state enable signal
   */
  void enableThermalMeasurement(bool state);

  /**
   * Start a thermal calibration
   * @param[in] window number of thermal frames used for averaging
   * @return error code
   */
  bool startThermalCalibration(std::size_t window);

  /**
   * Stop any ongoing thermal calibration
   * @return error code
   */
  bool stopThermalCalibration();

  /**
   * Set the light mode and color of the sensor ring
   * @param[in] mode Light mode to set
   * @param[in] red Red color value
   * @param[in] green Green color value
   * @param[in] blue Blue color value
   * @return true if successful, false otherwise
   */
  void setLight(light::LightMode mode, std::uint8_t red = 0, std::uint8_t green = 0, std::uint8_t blue = 0);

  /**
   * Factory method to build a MeasurementManager object
   * @param[in] params Parameter set for the MeasurementManager
   * @return Fully assembled MeasurementManager object
   */
  static std::unique_ptr<MeasurementManager> buildManager(ManagerParams params);

private:
  MeasurementManager(std::unique_ptr<MeasurementManagerImpl> mm_impl);

  std::unique_ptr<MeasurementManagerImpl> _mm_impl;
};

} // namespace manager

} // namespace eduart