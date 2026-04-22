// Copyright (c) 2026 EduArt Robotik GmbH

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
#include <thread>
#include <unordered_map>

#include "sensorring/SensorRing.hpp"
#include "sensorring/device/DepthSensor.hpp"
#include "sensorring/device/Group.hpp"
#include "sensorring/device/Light.hpp"
#include "sensorring/device/ThermalSensor.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
#include "sensorring/manager/ManagerParams.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/subscription/Publisher.hpp"

namespace eduart {

namespace sensorring {

namespace manager {

/**
 * @class MeasurementManagerImpl
 * @brief Implementation of MeasurementManager.
 */
class MeasurementManagerImpl {
public:
  MeasurementManagerImpl(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring);
  ~MeasurementManagerImpl() noexcept;

  bool measureSome() noexcept;
  bool startMeasuring() noexcept;
  bool stopMeasuring() noexcept;
  bool isMeasuring() noexcept;

  subscription::Subscription subscribeToStateChanges(std::function<void(const ManagerState state)> callback);

  ManagerState getManagerState() const noexcept;
  ManagerParams getParams() const noexcept;

  device::Group<device::DepthSensor> depthSensors() const noexcept;
  device::Group<device::ThermalSensor> thermalSensors() const noexcept;
  device::Group<device::Light> lights() const noexcept;

private:
  enum class MeasurementState {
    init,
    reset_sensors,
    sync_lights,
    get_eeprom,
    pre_loop_init,
    device_actions,
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

  void publishDepthMeasurements();
  void publishThermalMeasurements();
  void notifyState(const ManagerState state);

  const ManagerParams _params;
  std::atomic<ManagerState> _manager_state;
  std::atomic<MeasurementState> _measurement_state;
  std::unique_ptr<ring::SensorRing> _sensor_ring;

  bool _first_measurement;
  std::chrono::duration<double> _tof_measurement_period;
  std::chrono::duration<double> _thermal_measurement_period;
  std::chrono::time_point<std::chrono::steady_clock> _last_tof_measurement_timestamp;
  std::chrono::time_point<std::chrono::steady_clock> _last_thermal_measurement_timestamp;

  bool _is_tof_throttled;
  bool _is_thermal_throttled;
  bool _thermal_measurement_flag;

  std::atomic<bool> _is_running;
  std::thread _worker_thread;
  std::exception_ptr worker_exception;

  std::unordered_map<MeasurementFutureKey, std::future<bool>, MeasurementFutureKeyHash> _measurement_futures;

  subscription::Publisher<const ManagerState> _state_publisher;

  // Typed device vectors — abstract interfaces for public API
  std::vector<device::DepthSensor*> _depth_sensors;
  std::vector<device::ThermalSensor*> _thermal_sensors;
  std::vector<device::Light*> _lights;

  // Concrete device vectors — for internal CAN bus operations
  std::vector<device::VL53L8CX_Device*> _vl53l8cx_devices;
  std::vector<device::HTPA32_Device*> _htpa32_devices;
};

} // namespace manager

} // namespace sensorring

} // namespace eduart
