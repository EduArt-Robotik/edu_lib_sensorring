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
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "sensorring/Parameter.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/device/DeviceGroup.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/types/SubscriberToken.hpp"

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
   * @brief Subscribe to state changes; callback is invoked when the state changes.
   * @param[in] callback Invoked with the updated ManagerState.
   * @return Token to pass to unsubscribe.
   */
  SubscriberToken subscribeToStateChanges(std::function<void(const ManagerState state)> callback);

  /**
   * @brief Subscribe to device group updates; callback is invoked when the group is updated.
   * @param[in] key Device group to subscribe to.
   * @param[in] callback Invoked with the updated DeviceGroup.
   * @return Token to pass to unsubscribe.
   */
  SubscriberToken subscribeToDeviceGroup(device::DeviceType key, std::function<void(const device::DeviceGroup&)> callback);

  /**
   * @brief Cancel a subscription (state or device group).
   * @param[in] token Token returned by subscribeToStateChanges or subscribeToDeviceGroup.
   */
  void unsubscribe(SubscriberToken token);

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
  using Mutex     = std::mutex;
  using LockGuard = std::lock_guard<Mutex>;

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

  struct DeviceTypeHash {
    std::size_t operator()(device::DeviceType key) const noexcept { return static_cast<std::size_t>(key); }
  };

  struct MeasurementFutureKeyHash {
    std::size_t operator()(MeasurementFutureKey key) const noexcept { return static_cast<std::size_t>(key); }
  };

  void StateMachine();
  void StateMachineWorker() noexcept;

  bool waitForMeasurementFuture(MeasurementFutureKey key, std::chrono::steady_clock::duration timeout) noexcept;

  int notifyVL53L8CX();
  int notifyHTPA32();
  void notifyWS2812B();
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

  Mutex _extra_actions_mutex;
  std::queue<std::function<void()> > _extra_actions;

  std::atomic<bool> _is_running;
  std::thread _worker_thread;
  std::exception_ptr worker_exception;

  std::unordered_map<device::DeviceType, device::DeviceGroup, DeviceTypeHash> _device_groups;

  std::unordered_map<MeasurementFutureKey, std::future<bool>, MeasurementFutureKeyHash> _measurement_futures;

  mutable Mutex _subscriber_mutex;
  std::unordered_map<SubscriberToken, std::function<void(const ManagerState state)> > _state_subscriptions;
  std::unordered_map<device::DeviceType, std::unordered_map<SubscriberToken, std::function<void(const device::DeviceGroup&)> >, DeviceTypeHash> _device_subscriptions;
};

} // namespace manager

} // namespace eduart