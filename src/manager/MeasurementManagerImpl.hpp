// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementManagerImpl.hpp
 * @author EduArt Robotik GmbH
 * @brief  Implementation of MeasurementManager; holds state machine and private members.
 * @date   2025-02-15
 */

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

#include "sensorring/manager/MeasurementClient.hpp"
#include "sensorring/manager/ManagerSubscription.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/device/DeviceGroup.hpp"

namespace eduart {

namespace manager {

/**
 * @class MeasurementManagerImpl
 * @brief Implementation of MeasurementManager.
 */
class MeasurementManagerImpl {
public:
  /**
   * @brief Construct the implementation with parameters and owned SensorRing.
   * @param[in] params Manager configuration.
   * @param[in] sensor_ring SensorRing instance to manage (ownership transferred).
   */
  MeasurementManagerImpl(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring);

  /// Destructor
  ~MeasurementManagerImpl() noexcept;

  /**
   * @brief Run one processing cycle of the state machine worker.
   * @return true on success.
   */
  bool measureSome() noexcept;

  /**
   * @brief Start the state machine worker loop in a dedicated thread.
   * @return true on success.
   */
  bool startMeasuring() noexcept;

  /**
   * @brief Stop the state machine worker loop and join the thread.
   * @return true on success.
   */
  bool stopMeasuring() noexcept;

  /**
   * @brief Report whether the measurement worker thread is running.
   * @return true if the measurement thread is running.
   */
  bool isMeasuring() noexcept;

  /**
   * @brief Register a client to receive state and measurement callbacks.
   * @param[in] client Client to register; receives future notifications.
   */
  void registerClient(MeasurementClient* client);

  /**
   * @brief Unregister a client; it will no longer receive notifications.
   * @param[in] client Client to unregister.
   */
  void unregisterClient(MeasurementClient* client);

  /**
   * @brief Subscribe to device group updates; callback is invoked when the group is updated.
   * @param[in] key Device group to subscribe to.
   * @param[in] callback Invoked with the updated DeviceGroup.
   * @return Token to pass to unsubscribeFromDeviceGroup.
   */
  SubscriptionToken subscribeToDeviceGroup(DeviceGroupKey key, std::function<void(const device::DeviceGroup&)> callback);

  /**
   * @brief Cancel a device group subscription.
   * @param[in] token Token returned by subscribeToDeviceGroup.
   */
  void unsubscribeFromDeviceGroup(SubscriptionToken token);

  /**
   * @brief Return the current health state of the state machine worker.
   * @return Current manager state.
   */
  ManagerState getManagerState() const noexcept;

  /**
   * @brief Return the parameters used to initialize the manager.
   * @return Initial parameter struct.
   */
  ManagerParams getParams() const noexcept;

  /**
   * @brief Return the SensorRing managed by this implementation.
   * @return Pointer to the managed SensorRing (never null while implementation is alive).
   */
  ring::SensorRing* getSensorRing() const noexcept;

  /**
   * @brief Queue a callable to run once in the next extra-actions slot; executed from measurement thread; exceptions are caught and logged.
   * @param[in] action Callable executed once; should be non-blocking and exception-safe.
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

  struct DeviceGroupKeyHash {
    std::size_t operator()(DeviceGroupKey key) const noexcept { return static_cast<std::size_t>(key); }
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

  std::unordered_map<DeviceGroupKey, device::DeviceGroup, DeviceGroupKeyHash> _device_groups;
};

} // namespace manager

} // namespace eduart