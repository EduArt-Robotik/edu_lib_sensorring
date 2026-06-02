// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   MeasurementManagerImpl.hpp
 * @author EduArt Robotik GmbH
 * @brief  Implementation of MeasurementManager; holds tick-based scheduler and private members.
 * @date   2025-02-15
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "sensorring/SensorRing.hpp"
#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/light/Light.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"
#include "sensorring/device/thermal/ThermalSensor.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/types/Group.hpp"
#include "sensorring/manager/ManagerParams.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/subscription/Publisher.hpp"

#include "SchedulerTypes.hpp"

namespace eduart {

namespace sensorring {

namespace manager {

/**
 * @class MeasurementManagerImpl
 * @brief Implementation of MeasurementManager with tick-based scheduler.
 *
 * The scheduler runs at a fixed base rate. Each sensor group has an integer
 * divisor determining how often it fires. Within each tick:
 *   1. Wait for pending data from previous requests (self-regulating).
 *   2. Request new measurements for groups due this tick.
 *   3. Fetch data from groups that have pending results.
 *   4. Execute device actions (actuators).
 *   5. Sleep until next tick boundary.
 */
class MeasurementManagerImpl {
public:
  MeasurementManagerImpl(ManagerParams params, std::unique_ptr<SensorRing> sensor_ring);
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

  SensorRing* getRing() const noexcept;

private:
  enum class Phase {
    init,
    reset_sensors,
    configure_devices,
    get_eeprom,
    pre_loop_init,
    tick,
    error_handler_measurement,
    error_handler_communication,
    shutdown
  };

  void runPhase();
  void runWorker() noexcept;

  // Tick sub-steps
  bool waitForPendingData();
  bool fetchPendingData();
  void requestMeasurements();
  void executeDeviceActions();

  void publishDepthMeasurements();
  void publishThermalMeasurements();
  void notifyState(const ManagerState state);

  void buildSchedule();
  bool isGroupDue(const SensorGroupSchedule& group) const;

  const ManagerParams _params;
  std::atomic<ManagerState> _manager_state;
  Phase _phase;
  std::unique_ptr<SensorRing> _sensor_ring;

  // Scheduler state
  double _base_rate_hz;
  std::chrono::duration<double> _tick_period;
  std::chrono::time_point<std::chrono::steady_clock> _next_tick_time;
  unsigned long _tick_count;
  std::vector<SensorGroupSchedule> _schedule;

  // Pending futures for data-available signals (one per ToF family).
  std::future<bool> _vl53_data_available_future;
  std::future<bool> _tmf_data_available_future;

  std::atomic<bool> _is_running;
  std::thread _worker_thread;

  subscription::Publisher<const ManagerState> _state_publisher;

  // Typed device vectors — abstract interfaces for public API
  std::vector<device::DepthSensor*> _depth_sensors;
  std::vector<device::ThermalSensor*> _thermal_sensors;
  std::vector<device::Light*> _lights;

  // Concrete device vectors — for internal CAN bus operations
  std::vector<device::VL53L8CX_Device*> _vl53l8cx_devices;
  std::vector<device::TMF8829_Device*> _tmf8829_devices;
  std::vector<device::HTPA32_Device*> _htpa32_devices;
};

} // namespace manager

} // namespace sensorring

} // namespace eduart
