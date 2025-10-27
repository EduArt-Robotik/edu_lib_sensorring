#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <thread>

#include "MeasurementClient.hpp"
#include "Parameters.hpp"
#include "SensorRing.hpp"

namespace eduart {

namespace manager {

/**
 * @class MeasurementManagerImpl
 * @brief Implementation class of the MeasurementManager to hide private members.
 * @author Hannes Duske
 * @date 21.10.2025
 */
class MeasurementManagerImpl {
public:
  /**
   * Constructor
   * @param[in] params Parameter structure of the MeasurementManager
   */
  MeasurementManagerImpl(ManagerParams params);

  /**
   * Destructor
   */
  ~MeasurementManagerImpl();

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

private:
  enum class MeasurementState {
    init,
    reset_sensors,
    enumerate_sensors,
    sync_lights,
    get_eeprom,
    pre_loop_init,
    set_lights,
    request_tof_measurement,
    fetch_tof_data,
    request_thermal_measurement,
    fetch_thermal_data,
    wait_for_data,
    throttle_measurement,
    error_handler,
    shutdown
  };

  void init();
  void StateMachine();
  void StateMachineWorker();

  int notifyToFData();
  int notifyThermalData();
  void notifyState(const WorkerState state);

  const ManagerParams _params;
  WorkerState _manager_state;
  MeasurementState _measurement_state;
  std::unique_ptr<ring::SensorRing> _sensor_ring;

  bool _tof_enabled;
  bool _thermal_enabled;
  bool _first_measurement;
  std::chrono::time_point<std::chrono::steady_clock> _last_tof_measurement_timestamp_s;
  std::chrono::time_point<std::chrono::steady_clock> _last_thermal_measurement_timestamp_s;

  bool _is_tof_throttled;
  bool _is_thermal_throttled;
  bool _thermal_measurement_flag;

  light::LightMode _light_mode;
  std::uint8_t _light_color[3];
  std::uint8_t _light_brightness;
  std::atomic<bool> _light_update_flag;

  std::set<MeasurementClient*> _observers;

  std::atomic<bool> _is_running;
  std::thread _worker_thread;
};

} // namespace manager

} // namespace eduart