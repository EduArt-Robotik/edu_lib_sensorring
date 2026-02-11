#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "sensorring/MeasurementClient.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/device/DeviceGroup.hpp"

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
  MeasurementManagerImpl(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring);

  /**
   * Destructor
   */
  ~MeasurementManagerImpl() noexcept;

  /**
   * Run one processing cycle of the state machine worker
   * @return error code
   */
  bool measureSome() noexcept;

  /**
   * Start running the state machine worker loop in a thread
   * @return error code
   */
  bool startMeasuring() noexcept;

  /**
   * Stop the state machine worker loop and close the thread
   * @return error code
   */
  bool stopMeasuring() noexcept;

  /**
   * Query if the measurement thread is currently running
   * @return true if the measurement thread is running
   */
  bool isMeasuring() noexcept;

  /**
   * Register an client with the MeasurementManager object
   * @param[in] client Observer that is registered and gets notified on future events
   */
  void registerClient(MeasurementClient* client);

  /**
   * Unregister an client with the MeasurementManager object
   * @param[in] client Observer that is unregistered and will not be notified on future events
   */
  void unregisterClient(MeasurementClient* client);

  /**
   * Get the health status of the state machine
   * @return Current worker state
   */
  ManagerState getManagerState() const noexcept;

  /**
   * Get the parameters with which the MeasurementManager was initialized
   * @return Initial parameter struct
   */
  ManagerParams getParams() const noexcept;

  /**
   * Get the SensorRing managed by this implementation.
   * @return Pointer to the managed SensorRing
   */
  ring::SensorRing* getSensorRing() const noexcept;

  /**
   * Queue an extra action that will be executed in the dedicated
   * extra actions slot of the internal state machine.
   *
   * The action is executed from the measurement thread (or from the
   * thread calling measureSome(), respectively). The callable should
   * therefore be non-blocking and exception safe; any exception will
   * be caught and logged.
   *
   * @param[in] action callable to be executed once in the next
   *                   extra actions slot
   */
  void enqueueExtraAction(std::function<void()> action);

private:
  enum class MeasurementState {
    init,
    reset_sensors,
    enumerate_sensors,
    sync_lights,
    get_eeprom,
    pre_loop_init,
    extra_actions,
    request_tof_measurement,
    fetch_tof_data,
    request_thermal_measurement,
    fetch_thermal_data,
    wait_for_data,
    throttle_measurement,
    error_handler_measurement,
    error_handler_communication,
    shutdown
  };

  enum class MeasurementFutureKey {
    ToFRequest,
    ThermalRequest
  };

  struct MeasurementFutureKeyHash {
    std::size_t operator()(MeasurementFutureKey key) const noexcept { return static_cast<std::size_t>(key); }
  };

  void StateMachine();
  void StateMachineWorker() noexcept;

  bool waitForMeasurementFuture(MeasurementFutureKey key, std::chrono::steady_clock::duration timeout) noexcept;

  int notifyToFData();
  int notifyThermalData();
  void notifyState(const ManagerState state);

  const ManagerParams _params;
  std::atomic<ManagerState> _manager_state;
  std::atomic<MeasurementState> _measurement_state;
  std::unique_ptr<ring::SensorRing> _sensor_ring;

  bool _tof_enabled;
  bool _thermal_enabled;
  bool _first_measurement;
  std::chrono::duration<double> _tof_measurement_period;
  std::chrono::duration<double> _thermal_measurement_period;
  std::chrono::time_point<std::chrono::steady_clock> _last_tof_measurement_timestamp;
  std::chrono::time_point<std::chrono::steady_clock> _last_thermal_measurement_timestamp;

  bool _is_tof_throttled;
  bool _is_thermal_throttled;
  bool _thermal_measurement_flag;

  mutable std::mutex _client_mutex;
  using LockGuard = std::lock_guard<std::mutex>;
  std::set<MeasurementClient*> _clients;

  std::mutex _extra_actions_mutex;
  std::queue<std::function<void()> > _extra_actions;

  std::atomic<bool> _is_running;
  std::thread _worker_thread;
  std::exception_ptr worker_exception;

  std::unordered_map<MeasurementFutureKey, std::future<bool>, MeasurementFutureKeyHash> _measurement_futures;

  device::DeviceGroup _tof_device_group;
  device::DeviceGroup _thermal_device_group;
  device::DeviceGroup _light_device_group;
};

} // namespace manager

} // namespace eduart