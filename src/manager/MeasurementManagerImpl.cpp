#include "manager/MeasurementManagerImpl.hpp"

#include "interface/ComInterface.hpp"
#include "sensorring/SensorBus.hpp"
#include "sensorring/board/SensorBoard.hpp"
#include "sensorring/device/Device.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace std::chrono_literals;

namespace eduart {

namespace sensorring {

namespace manager {

MeasurementManagerImpl::MeasurementManagerImpl(ManagerParams params, std::unique_ptr<SensorRing> sensor_ring)
    : _params(params)
    , _manager_state(ManagerState::Uninitialized)
    , _phase(Phase::init)
    , _sensor_ring(std::move(sensor_ring))
    , _base_rate_hz(0.0)
    , _tick_period(0.0)
    , _next_tick_time(std::chrono::steady_clock::now())
    , _tick_count(0)
    , _is_running(false)
    , _depth_publish_needed(false)
    , _thermal_publish_needed(false)
    , _phase_deadline(std::chrono::steady_clock::now())
    , _error_attempts(0)
    , _repair_success(false)
    , _action_queue(std::make_unique<ActionQueue>()) {

  if (!_sensor_ring) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "MeasurementManager got passed an invalid SensorRing.");
    _phase = Phase::shutdown;
    return;
  }

  if (_sensor_ring->getDevices().empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "MeasurementManager got passed an empty SensorRing without any devices.");
    _phase = Phase::shutdown;
    return;
  }

  if (_params.timeout == 0ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "MeasurementManager got passed a SensorRing timeout parameter of 0.0s");
  } else if (_params.timeout < 200ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "MeasurementManager got passed a SensorRing timeout parameter of " + std::to_string(_params.timeout.count()) + " ms, which is probably too low");
  }

  // Populate typed device vectors via dynamic_cast.
  for (auto* dev : _sensor_ring->getDevices()) {
    if (auto* ds = dynamic_cast<device::DepthSensor*>(dev))
      _depth_sensors.push_back(ds);
    if (auto* ts = dynamic_cast<device::ThermalSensor*>(dev))
      _thermal_sensors.push_back(ts);
    if (auto* lt = dynamic_cast<device::Light*>(dev))
      _lights.push_back(lt);
    if (auto* vl = dynamic_cast<device::VL53L8CX_Device*>(dev))
      _vl53l8cx_devices.push_back(vl);
    if (auto* tmf = dynamic_cast<device::TMF8829_Device*>(dev))
      _tmf8829_devices.push_back(tmf);
    if (auto* ht = dynamic_cast<device::HTPA32_Device*>(dev))
      _htpa32_devices.push_back(ht);
  }

  // Populate action queues for managed operation.
  ActionQueue::setGlobalQueue(_action_queue.get());
  for (auto* dev : _sensor_ring->getDevices()) {
    dev->setQueue(_action_queue.get());
  }

  buildSchedule();

  _manager_state = ManagerState::Initialized;
}

MeasurementManagerImpl::~MeasurementManagerImpl() noexcept {
  stopMeasuring();
}

ManagerParams MeasurementManagerImpl::getParams() const noexcept {
  return _params;
}

/* =======================================================================================
        Schedule computation
==========================================================================================
*/

void MeasurementManagerImpl::buildSchedule() {
  _schedule.clear();

  // Build one group per configured device type.
  if (!_vl53l8cx_devices.empty()) {
    SensorGroupSchedule group;
    group.type = device::DeviceType::VL53L8CX;
    // Group max rate is limited by the slowest sensor in the group.
    group.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _vl53l8cx_devices) {
      group.max_rate_hz = std::min(group.max_rate_hz, dev->getParams().max_rate_hz);
      group.sensors.push_back(dev); // Adding sensor to the group
    }
    _schedule.push_back(std::move(group));
  }

  if (!_tmf8829_devices.empty()) {
    SensorGroupSchedule group;
    group.type = device::DeviceType::TMF8829;
    // Group max rate is limited by the slowest sensor in the group.
    group.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _tmf8829_devices) {
      group.max_rate_hz = std::min(group.max_rate_hz, dev->getParams().max_rate_hz);
      group.sensors.push_back(dev);
    }
    _schedule.push_back(std::move(group));
  }

  if (!_htpa32_devices.empty()) {
    SensorGroupSchedule group;
    group.type        = device::DeviceType::HTPA32;
    group.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _htpa32_devices) {
      group.max_rate_hz = std::min(group.max_rate_hz, dev->getParams().max_rate_hz);
      group.sensors.push_back(dev);
    }
    _schedule.push_back(std::move(group));
  }

  if (_schedule.empty() && !_lights.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "No sensors found in the SensorRing. Only lights will be scheduled.");

    SensorGroupSchedule group;
    group.type        = device::DeviceType::WS2812b;
    group.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _lights) {
      group.max_rate_hz = std::min(group.max_rate_hz, dev->getParams().max_rate_hz);
    }
    _schedule.push_back(std::move(group));
  }

  _base_rate_hz = computeScheduleFastest(_schedule);
  //_base_rate_hz = computeScheduleCommonMultiple(_schedule);

  if (_base_rate_hz > 0.0) {
    _tick_period = std::chrono::duration<double>(1.0 / _base_rate_hz);
  } else {
    _base_rate_hz = FALLBACK_LOOP_RATE_HZ;
    _tick_period  = std::chrono::duration<double>(1.0 / FALLBACK_LOOP_RATE_HZ);
  }

  logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Scheduler: base rate = " + std::to_string(_base_rate_hz) + " Hz, tick period = " + std::to_string(_tick_period.count() * 1000.0) + " ms");
  for (const auto& g : _schedule) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "  Group " + device::toString(g.type) + ": divisor = " + std::to_string(g.divisor) + ", effective rate = " + std::to_string(g.effective_rate_hz) + " Hz");
  }
}

bool MeasurementManagerImpl::isGroupDue(const SensorGroupSchedule& group) const {
  return (_tick_count % group.divisor) == 0;
}

/* =======================================================================================
        Subscriptions
==========================================================================================
*/

subscription::Subscription MeasurementManagerImpl::subscribeToStateChanges(std::function<void(ManagerState state)> callback) {
  return _state_publisher.subscribe(std::move(callback));
}

void MeasurementManagerImpl::publishDepthMeasurements() {
  for (auto* sensor : _depth_sensors) {
    sensor->publishMeasurement();
  }
}

void MeasurementManagerImpl::publishThermalMeasurements() {
  for (auto* sensor : _thermal_sensors) {
    sensor->publishMeasurement();
  }
}

void MeasurementManagerImpl::notifyState(ManagerState state) {
  _state_publisher.publish(state);
}

ManagerState MeasurementManagerImpl::getManagerState() const noexcept {
  return _manager_state;
}

/* =======================================================================================
        Start and stop measurements
==========================================================================================
*/

bool MeasurementManagerImpl::measureSome() noexcept {
  if (_is_running) {
    // Worker thread is active — use stopMeasuring() before calling measureSome().
    return false;
  }

  try {
    runPhase();
    return _phase != Phase::shutdown;
  } catch (const std::exception& e) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in scheduler: " + std::string(e.what()));
    _phase = Phase::error_comm_enter;
    return false;
  }
}

bool MeasurementManagerImpl::startMeasuring() noexcept {
  if (!_is_running) {
    _is_running    = true;
    _worker_thread = std::thread(&MeasurementManagerImpl::runWorker, this);
    notifyState(ManagerState::Running);
    return true;
  }

  return false;
}

bool MeasurementManagerImpl::stopMeasuring() noexcept {
  if (_is_running) {
    _is_running = false;
    _worker_wait_cv.notify_all();
    notifyState(ManagerState::Shutdown);
  }

  if (_worker_thread.joinable()) {
    _worker_thread.join();
    return true;
  }

  return false;
}

bool MeasurementManagerImpl::isMeasuring() noexcept {
  return _is_running;
}

/* =======================================================================================
        Scheduler phases
==========================================================================================
*/

void MeasurementManagerImpl::runWorker() noexcept {
  while (_is_running) {
    try {
      if (!runPhase()) {
        const auto wait_duration = computeWorkerIdleWait();
        if (wait_duration <= std::chrono::steady_clock::duration::zero()) {
          std::this_thread::yield();
          continue;
        }

        std::unique_lock<std::mutex> lock(_worker_wait_mutex);
        _worker_wait_cv.wait_for(lock, wait_duration, [this]() {
          return !_is_running.load();
        });
      }
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in scheduler: " + std::string(e.what()));
      _phase = Phase::error_comm_enter;
    }
  }
}

std::chrono::steady_clock::duration MeasurementManagerImpl::computeWorkerIdleWait() const noexcept {
  using clock = std::chrono::steady_clock;

  static constexpr auto kMinPollWait    = std::chrono::milliseconds(1);
  static constexpr auto kMaxTimerWait   = std::chrono::milliseconds(10);
  static constexpr auto kImmediateRetry = clock::duration::zero();

  const auto now = clock::now();

  auto clampTimerWait = [&](const clock::time_point deadline) -> clock::duration {
    if (deadline <= now) {
      return kImmediateRetry;
    }
    const auto remaining = deadline - now;
    return std::min(remaining, std::chrono::duration_cast<clock::duration>(kMaxTimerWait));
  };

  switch (_phase) {
  case Phase::reset_sensors_wait:
  case Phase::error_comm_wait:
    return clampTimerWait(_phase_deadline);

  case Phase::tick_sleep:
    return clampTimerWait(_next_tick_time);

  case Phase::tick_wait_pending:
  case Phase::tick_fetch_wait:
  case Phase::error_meas_wait:
    // Futures may complete at any time; keep polls frequent but do not spin.
    return std::chrono::duration_cast<clock::duration>(kMinPollWait);

  default:
    return kImmediateRetry;
  }
}

bool MeasurementManagerImpl::runPhase() {
  switch (_phase) {

    /* =============================================
      Initialization sequence
    ============================================= */

  case Phase::init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManager scheduler");
    _phase = Phase::reset_sensors;
    return true;
  }

  case Phase::reset_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
    board::SensorBoard::resetBoards();
    _phase_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    _phase          = Phase::reset_sensors_wait;
    return true;
  }

  case Phase::reset_sensors_wait: {
    if (std::chrono::steady_clock::now() < _phase_deadline) {
      return false;
    }
    _phase = Phase::sync_lights;
    return true;
  }

  case Phase::sync_lights: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Synchronizing lights");
    device::WS2812b_Device::syncLight();
    _phase = Phase::configure_interfaces;
    return true;
  }

  case Phase::configure_interfaces: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Configuring interfaces after reset");

    bool success = true;
    for (const auto& bus : _sensor_ring->getSensorBuses()) {
      success &= bus->getInterface()->configure();
    }

    if (success) {
      _phase = Phase::configure_devices;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to configure at least one interface after reset.");
      _phase = Phase::shutdown;
    }
    return true;
  }

  case Phase::configure_devices: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Configuring devices after reset");

    bool success = true;
    for (auto bus : _sensor_ring->getSensorBuses()) {
      for (auto board : bus->getSensorBoards()) {
        success &= board->configure();
        if (!success)
          break;
      }
    }

    if (success) {
      _phase = Phase::pre_loop_init;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to configure at least one device after reset.");
      _phase = Phase::shutdown;
    }
    return true;
  }

  case Phase::pre_loop_init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Starting measurement loop.");
    _tick_count     = 0;
    _next_tick_time = std::chrono::steady_clock::now();

    for (auto& g : _schedule) {
      g.has_pending_request = false;
      g.has_fetch_ready     = false;
    }

    notifyState(ManagerState::Running);
    _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
    _phase          = Phase::tick_wait_pending;
    return true;
  }

    /* =============================================
      Tick-based measurement loop
    ============================================= */

  case Phase::tick_wait_pending: {
    // Non-blocking poll: check pending measurements only when their group is due.
    // Slower groups keep acquiring across intervening ticks.
    for (auto& group : _schedule) {
      if (!group.has_pending_request || !isGroupDue(group)) {
        continue;
      }

      for (auto& future : group.measurement_futures) {
        if (!future.valid()) {
          continue;
        }
        if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
          if (std::chrono::steady_clock::now() > _phase_deadline) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout waiting for " + device::toString(group.type) + " measurement completion.");
            _phase = Phase::error_meas_enter;
          }
          return false;
        }
        if (!future.get()) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, device::toString(group.type) + " measurement request failed.");
          _phase = Phase::error_meas_enter;
          return false;
        }
      }
      group.measurement_futures.clear();

      // Data is confirmed available — promote this group to fetch-ready.
      group.has_fetch_ready     = true;
      group.has_pending_request = false;
    }

    // All pending groups due this tick are ready — advance.
    _phase = Phase::tick_request;
    return true;
  }

  case Phase::tick_request: {
    // Fire new measurement requests for groups due this tick so sensor hardware
    // starts its next cycle while we are transferring the previous cycle's data.
    requestMeasurements();

    prepareFetchSensors();
    if (_fetch_sensors.empty()) {
      for (auto& group : _schedule) {
        group.has_fetch_ready = false;
      }
      _phase = Phase::tick_actions;
      return true;
    }

    launchFetchBatch();
    _phase = Phase::tick_fetch_wait;
    return true;
  }

  case Phase::tick_fetch_wait: {
    for (auto& future : _fetch_futures) {
      if (future.valid() && future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        if (std::chrono::steady_clock::now() > _phase_deadline) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout fetching measurement data.");
          _fetch_futures.clear();
          _fetch_sensors.clear();
          _phase = Phase::error_meas_enter;
        }
        return false;
      }
    }

    bool success = true;
    for (auto& future : _fetch_futures) {
      if (future.valid()) {
        success &= future.get();
      }
    }
    _fetch_futures.clear();

    if (!success) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Fetching measurement data failed.");
      _fetch_sensors.clear();
      _phase = Phase::error_meas_enter;
      return true;
    }

    _fetch_index = _fetch_batch_end;
    if (_fetch_index == _tof_fetch_count && _depth_publish_needed) {
      publishDepthMeasurements();
      _depth_publish_needed = false;
    }

    if (_fetch_index < _fetch_sensors.size()) {
      launchFetchBatch();
      return true;
    }

    if (_thermal_publish_needed) {
      publishThermalMeasurements();
      _thermal_publish_needed = false;
    }

    _fetch_sensors.clear();
    _fetch_index     = 0;
    _tof_fetch_count = 0;
    for (auto& g : _schedule) {
      g.has_fetch_ready = false;
    }

    _phase = Phase::tick_actions;
    return true;
  }

  case Phase::tick_actions: {
    _action_queue->processAll();

    _tick_count++;
    _next_tick_time += std::chrono::duration_cast<std::chrono::steady_clock::duration>(_tick_period);

    _phase = Phase::tick_sleep;
    return true;
  }

  case Phase::tick_sleep: {
    if (std::chrono::steady_clock::now() < _next_tick_time) {
      return false;
    }

    // Set the data-wait deadline for the upcoming tick_wait_pending phase.
    _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
    _phase          = Phase::tick_wait_pending;
    return true;
  }

    /* =============================================
      Measurement error recovery
    ============================================= */

  case Phase::error_meas_enter: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Measurement error handler called.");
    notifyState(ManagerState::Error);

    if (!_params.repair_errors) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Shutting down: parameter \"repair_errors\" is false.");
      _phase = Phase::shutdown;
      return true;
    }

    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Attempting to restart measurements.");
    _error_attempts = 0;
    _phase          = Phase::error_meas_retry;
    return true;
  }

  case Phase::error_meas_retry: {
    if (_error_attempts >= 10) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Measurement restart failed after " + std::to_string(_error_attempts) + " attempt(s). Resetting all sensors.");
      _phase = Phase::reset_sensors;
      return true;
    }

    // Reset sensor state for a clean retry.
    for (auto* dev : _depth_sensors)
      dev->resetSensorState();
    for (auto* dev : _thermal_sensors)
      dev->resetSensorState();
    for (auto& g : _schedule) {
      g.has_pending_request = false;
      g.has_fetch_ready     = false;
    }

    _tick_count = 0;
    requestMeasurements();
    _error_attempts++;

    _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
    _phase          = Phase::error_meas_wait;
    return true;
  }

  case Phase::error_meas_wait: {
    for (auto& group : _schedule) {
      if (!group.has_pending_request) {
        continue;
      }

      for (auto& future : group.measurement_futures) {
        if (!future.valid()) {
          continue;
        }
        if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
          if (std::chrono::steady_clock::now() > _phase_deadline) {
            _phase = Phase::error_meas_retry;
          }
          return false;
        }
        if (!future.get()) {
          _phase = Phase::error_meas_retry;
          return false;
        }
      }
      group.measurement_futures.clear();

      group.has_pending_request = false;
    }

    // All pending futures resolved — restart succeeded.
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Measurement restart succeeded after " + std::to_string(_error_attempts) + " attempt(s).");
    _tick_count     = 0;
    _next_tick_time = std::chrono::steady_clock::now();
    for (auto& g : _schedule) {
      g.has_pending_request = false;
      g.has_fetch_ready     = false;
    }
    notifyState(ManagerState::Running);
    _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
    _phase          = Phase::tick_wait_pending;
    return true;
  }

    /* =============================================
      Communication error recovery
    ============================================= */

  case Phase::error_comm_enter: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Communication error handler called.");
    notifyState(ManagerState::Error);

    if (!_params.repair_errors) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Shutting down: parameter \"repair_errors\" is false.");
      _phase = Phase::shutdown;
      return true;
    }

    bool has_error = false;
    for (const auto& bus : _sensor_ring->getSensorBuses()) {
      has_error |= bus->getInterface()->hasError();
    }

    if (!has_error) {
      // No interface error found — nothing to repair; resume the measurement loop.
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "No communication error found on any interface. Resuming.");
      _tick_count     = 0;
      _next_tick_time = std::chrono::steady_clock::now();
      for (auto& g : _schedule) {
        g.has_pending_request = false;
        g.has_fetch_ready     = false;
      }
      notifyState(ManagerState::Running);
      _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
      _phase          = Phase::tick_wait_pending;
      return true;
    }

    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Communication error detected. Attempting to restart affected interfaces.");
    _error_attempts = 0;
    _repair_success = false;
    _phase          = Phase::error_comm_repair;
    return true;
  }

  case Phase::error_comm_repair: {
    if (_error_attempts >= 40) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Communication repair failed after " + std::to_string(_error_attempts) + " attempt(s). Please check the interfaces.");
      _phase = Phase::shutdown;
      return true;
    }

    // Attempt one repair pass over all faulty interfaces.
    _repair_success = true;
    for (const auto& bus : _sensor_ring->getSensorBuses()) {
      auto interface = bus->getInterface();
      if (interface->hasError()) {
        try {
          _repair_success &= interface->repairInterface();
        } catch (const std::runtime_error&) {
          _repair_success = false;
        }
      }
    }
    _error_attempts++;

    _phase_deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
    _phase          = Phase::error_comm_wait;
    return true;
  }

  case Phase::error_comm_wait: {
    if (std::chrono::steady_clock::now() < _phase_deadline) {
      return false;
    }

    if (_repair_success) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Communication repair succeeded after " + std::to_string(_error_attempts) + " attempt(s).");
      _tick_count     = 0;
      _next_tick_time = std::chrono::steady_clock::now();
      for (auto& g : _schedule) {
        g.has_pending_request = false;
        g.has_fetch_ready     = false;
      }
      notifyState(ManagerState::Running);
      _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
      _phase          = Phase::tick_wait_pending;
    } else {
      _phase = Phase::error_comm_repair;
    }
    return true;
  }

    /* =============================================
      Shutdown
    ============================================= */

  case Phase::shutdown: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Shutting down scheduler.");
    notifyState(ManagerState::Shutdown);
    _is_running = false;
    return true;
  }
  }

  return true;
}

/* =======================================================================================
        Tick sub-steps
==========================================================================================
*/

void MeasurementManagerImpl::prepareFetchSensors() {
  _fetch_sensors.clear();
  _fetch_index            = 0;
  _tof_fetch_count        = 0;
  _depth_publish_needed   = false;
  _thermal_publish_needed = false;

  for (auto& group : _schedule) {
    if (!group.has_fetch_ready) {
      continue;
    }
    if (group.type == device::DeviceType::VL53L8CX || group.type == device::DeviceType::TMF8829) {
      _depth_publish_needed = true;
    }
    if (group.type == device::DeviceType::HTPA32) {
      _tof_fetch_count        = _fetch_sensors.size();
      _thermal_publish_needed = true;
    }
    _fetch_sensors.insert(_fetch_sensors.end(), group.sensors.begin(), group.sensors.end());
  }

  if (!_thermal_publish_needed) {
    _tof_fetch_count = _fetch_sensors.size();
  }
}

void MeasurementManagerImpl::launchFetchBatch() {
  _fetch_futures.clear();
  _fetch_batch_end = (_fetch_index < _tof_fetch_count) ? _tof_fetch_count : _fetch_sensors.size();
  for (std::size_t i = _fetch_index; i < _fetch_batch_end; ++i) {
    _fetch_futures.emplace_back(_fetch_sensors[i]->fetch());
  }
  _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
}

void MeasurementManagerImpl::requestMeasurements() {
  for (auto& group : _schedule) {
    if (!isGroupDue(group) || group.sensors.empty()) {
      continue;
    }

    group.measurement_futures.clear();
    const auto sequence = group.measurement_sequence++;
    for (auto* sensor : group.sensors) {
      group.measurement_futures.emplace_back(sensor->measure(sequence));
    }
    group.has_pending_request = true;
  }
}

/* =======================================================================================
        Typed device accessors
==========================================================================================
*/

device::Group<device::DepthSensor> MeasurementManagerImpl::depthSensors() const noexcept {
  return device::Group<device::DepthSensor>(_depth_sensors);
}

device::Group<device::ThermalSensor> MeasurementManagerImpl::thermalSensors() const noexcept {
  return device::Group<device::ThermalSensor>(_thermal_sensors);
}

device::Group<device::Light> MeasurementManagerImpl::lights() const noexcept {
  return device::Group<device::Light>(_lights);
}

SensorRing* MeasurementManagerImpl::getRing() const noexcept {
  return _sensor_ring.get();
}

} // namespace manager

} // namespace sensorring

} // namespace eduart
