#pragma once

#include <memory>
#include <string>

#include "sensorring/utils/CustomTypes.hpp"
#include "sensorring/MeasurementClient.hpp"
#include "sensorring/Parameters.hpp"

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

// Forward declaration of implementation class
class SENSORRING_API MeasurementManagerImpl;

/**
 * @class MeasurementManager
 * @brief Meta class that handles the timing, triggering and processing of sensor measurements. Internally it runs a
 * looping state machine.
 * @author Hannes Duske
 * @date 25.12.2024
 */
class SENSORRING_API MeasurementManager {
public:
  /**
   * Constructor
   */
  MeasurementManager(ManagerParams params);

  /**
   * Destructor
   */
  ~MeasurementManager() noexcept;

  /**
   * Run one processing cycle of the state machine worker
   * @throw Throws std::runtime_error in case the measurement fails
   * @return true on success
   */
  bool measureSome();

  /**
   * Start running the state machine worker loop in a thread
   * @return true on success
   */
  bool startMeasuring() noexcept;

  /**
   * Stop the state machine worker loop and close the thread
   * @return true on success
   */
  bool stopMeasuring() noexcept;

  /**
   * Query if the measurement thread is currently running
   * @return true if the measurement thread is running
   */
  bool isMeasuring() noexcept;

  /**
   * Register an observer with the MeasurementManager object
   * @param[in] observer Observer that is registered and gets notified on future events
   */
  void registerClient(MeasurementClient* observer) noexcept;

  /**
   * Unregister an observer with the MeasurementManager object
   * @param[in] observer Observer that is unregistered and will not be notified on future events
   */
  void unregisterClient(MeasurementClient* observer) noexcept;

  /**
   * Get a string representation of the topology of the connected sensors
   * @return Formatted string describing the topology
   */
  std::string printTopology() const noexcept;

  /**
   * Get the health status of the state machine
   * @return Current manager state
   */
  ManagerState getManagerState() const noexcept;

  /**
   * Get the parameters with which the MeasurementManager was initialized
   * @return Initial parameter struct
   */
  ManagerParams getParams() const noexcept;

  /**
   * Enable or disable the Time-of-Flight sensor measurements
   * @param[in] state enable signal
   */
  void enableTofMeasurement(bool state) noexcept;

  /**
   * Enable or disable the thermal sensor measurements
   * @param[in] state enable signal
   */
  void enableThermalMeasurement(bool state) noexcept;

  /**
   * Start a thermal calibration
   * @param[in] window number of thermal frames used for averaging
   * @return true on success
   */
  bool startThermalCalibration(std::size_t window) noexcept;

  /**
   * Stop any ongoing thermal calibration
   * @return true on success
   */
  bool stopThermalCalibration() noexcept;

  /**
   * Set the light mode and color of the sensor ring
   * @param[in] mode Light mode to set
   * @param[in] red Red color value
   * @param[in] green Green color value
   * @param[in] blue Blue color value
   * @return true if successful, false otherwise
   */
  void setLight(light::LightMode mode, std::uint8_t red = 0, std::uint8_t green = 0, std::uint8_t blue = 0) noexcept;

private:
  std::unique_ptr<MeasurementManagerImpl> _mm_impl;
};

} // namespace manager

} // namespace eduart