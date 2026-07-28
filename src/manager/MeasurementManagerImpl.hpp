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
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
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
 * The scheduler is driven by a non-blocking state machine. Each call to
 * runPhase() advances the machine by at most one step and returns immediately
 * if the current phase is waiting for I/O or a timer.  This lets the same
 * implementation serve two modes of operation:
 *   - Threaded mode: runWorker() calls runPhase() in a tight loop; a brief
 *     yield prevents busy-spinning during wait phases.
 *   - Spin mode: the caller drives the loop by calling measureSome() at its
 *     own rate (e.g. from a ROS spin callback).
 *
 * The tick pipeline is split into five non-blocking sub-phases:
 *   1. tick_wait_pending  — poll data-available futures from previous tick.
 *   2. tick_request       — fire measurement requests + launch fetch futures.
 *   3. tick_fetch_wait    — poll fetch futures; publish when all done.
 *   4. tick_actions       — execute actuator actions; advance tick counter.
 *   5. tick_sleep         — poll tick boundary; advance when elapsed.
 */
class MeasurementManagerImpl {
public:
  MeasurementManagerImpl(ManagerParams params, std::unique_ptr<SensorRing> sensor_ring);
  ~MeasurementManagerImpl() noexcept;

  bool measureSome() noexcept;
  bool startMeasuring() noexcept;
  bool stopMeasuring() noexcept;
  bool isMeasuring() noexcept;

  subscription::Subscription subscribeToStateChanges(std::function<void(ManagerState state)> callback);

  ManagerState getManagerState() const noexcept;
  ManagerParams getParams() const noexcept;

  device::Group<device::DepthSensor> depthSensors() const noexcept;
  device::Group<device::ThermalSensor> thermalSensors() const noexcept;
  device::Group<device::Light> lights() const noexcept;

  SensorRing* getRing() const noexcept;

private:
  enum class Phase {
    // Initialization sequence
    init,
    reset_sensors,
    reset_sensors_wait, ///< Non-blocking delay after hardware reset (2 s).
    sync_lights,
    configure_interfaces,
    configure_devices,
    pre_loop_init,

    // Tick-based measurement loop (each sub-phase returns immediately when waiting)
    tick_wait_pending, ///< Poll data-available futures from previous tick.
    tick_request,      ///< Fire measurement requests; launch fetch futures.
    tick_fetch_wait,   ///< Poll fetch futures; publish measurements when done.
    tick_actions,      ///< Execute actuator actions; advance tick counter.
    tick_sleep,        ///< Non-blocking wait until next tick boundary.

    // Measurement error recovery
    error_meas_enter, ///< Log and notify; decide whether to attempt repair.
    error_meas_retry, ///< Reset sensor state and fire a fresh request.
    error_meas_wait,  ///< Poll data-available futures during recovery.

    // Communication error recovery
    error_comm_enter,  ///< Log and notify; check for interface errors.
    error_comm_repair, ///< Call repairInterface() for all faulty buses.
    error_comm_wait,   ///< Non-blocking inter-attempt delay (250 ms).

    shutdown
  };

  static constexpr double FALLBACK_LOOP_RATE_HZ = 10.0;

  /// Advance the state machine by one step. Returns true if the phase made
  /// progress (transitioned), false if it is still waiting (caller may yield).
  bool runPhase();
  std::chrono::steady_clock::duration computeWorkerIdleWait() const noexcept;
  void runWorker() noexcept;

  // Tick sub-steps
  void launchFetchFutures();
  void requestMeasurements();
  void executeDeviceActions();

  void publishDepthMeasurements();
  void publishThermalMeasurements();
  void notifyState(ManagerState state);

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
  std::atomic<bool> _is_running;

  // Pending futures for data-available signals (one per ToF family).
  std::future<bool> _vl53_data_available_future;
  std::future<bool> _tmf_data_available_future;

  // Fetch futures launched in tick_request, polled in tick_fetch_wait.
  std::vector<std::future<bool> > _vl53l8cx_fetch_futures;
  std::vector<std::future<bool> > _tmf8829_fetch_futures;
  std::vector<std::future<bool> > _htpa32_fetch_futures;

  // Tracks which measurement types need publishing after fetch completes.
  bool _depth_publish_needed;
  bool _thermal_publish_needed;

  // Deadline used by non-blocking wait phases (_phase_deadline) and error recovery.
  std::chrono::time_point<std::chrono::steady_clock> _phase_deadline;
  unsigned int _error_attempts;
  bool _repair_success;

  std::thread _worker_thread;
  mutable std::mutex _worker_wait_mutex;
  std::condition_variable _worker_wait_cv;

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
