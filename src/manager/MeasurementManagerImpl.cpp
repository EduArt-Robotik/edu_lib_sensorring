#include "manager/MeasurementManagerImpl.hpp"

#include "board/SensorBoardCommands.hpp"
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
    , _phase_deadline(std::chrono::steady_clock::now())
    , _error_attempts(0)
    , _repair_success(false)
    , _is_running(false) {

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
  class FunctionMeasurementGroupExecutor final : public MeasurementGroupExecutor {
  public:
    using RequestFn = std::function<std::future<bool>(std::chrono::milliseconds)>;
    using FetchFn   = std::function<void(std::chrono::milliseconds, std::vector<std::future<bool>>&)>;

    FunctionMeasurementGroupExecutor(SensorGroupSchedule schedule, PublishTarget publish_target, bool wait_for_request_ready, RequestFn request_fn, FetchFn fetch_fn)
        : _schedule(std::move(schedule))
        , _publish_target(publish_target)
        , _wait_for_request_ready(wait_for_request_ready)
        , _request_fn(std::move(request_fn))
        , _fetch_fn(std::move(fetch_fn)) {
    }

    SensorGroupSchedule& schedule() noexcept override {
      return _schedule;
    }

    const SensorGroupSchedule& schedule() const noexcept override {
      return _schedule;
    }

    PublishTarget getPublishTarget() const noexcept override {
      return _publish_target;
    }

    void clearCycleState() override {
      _schedule.has_pending_request = false;
      _schedule.has_fetch_ready     = false;
      _fetch_futures.clear();
    }

    bool requestMeasurement(std::chrono::milliseconds timeout) override {
      if (!_request_fn) {
        return false;
      }

      _request_future = _request_fn(timeout);
      return true;
    }

    AsyncPollResult pollRequestReady(std::chrono::time_point<std::chrono::steady_clock> deadline) override {
      if (!_wait_for_request_ready) {
        return AsyncPollResult::Ready;
      }

      if (!_request_future.valid()) {
        return AsyncPollResult::Failed;
      }

      if (_request_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        if (std::chrono::steady_clock::now() > deadline) {
          return AsyncPollResult::TimedOut;
        }
        return AsyncPollResult::Waiting;
      }

      return _request_future.get() ? AsyncPollResult::Ready : AsyncPollResult::Failed;
    }

    void launchFetch(std::chrono::milliseconds timeout) override {
      if (_fetch_fn) {
        _fetch_fn(timeout, _fetch_futures);
      }
    }

    AsyncPollResult pollFetchReady(std::chrono::time_point<std::chrono::steady_clock> deadline) override {
      for (auto& fut : _fetch_futures) {
        if (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
          if (std::chrono::steady_clock::now() > deadline) {
            return AsyncPollResult::TimedOut;
          }
          return AsyncPollResult::Waiting;
        }
      }

      return AsyncPollResult::Ready;
    }

    bool hasFetchWork() const noexcept override {
      return !_fetch_futures.empty();
    }

    bool collectFetchResults() override {
      bool success = true;
      for (auto& fut : _fetch_futures) {
        success &= fut.get();
      }
      _fetch_futures.clear();
      return success;
    }

  private:
    SensorGroupSchedule _schedule;
    PublishTarget _publish_target;
    bool _wait_for_request_ready;

    RequestFn _request_fn;
    FetchFn _fetch_fn;

    std::future<bool> _request_future;
    std::vector<std::future<bool>> _fetch_futures;
  };

  _group_executors.clear();

  // Build one executor group per configured device type.
  if (!_vl53l8cx_devices.empty()) {
    SensorGroupSchedule schedule;
    schedule.type        = device::DeviceType::VL53L8CX;
    schedule.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _vl53l8cx_devices) {
      schedule.max_rate_hz = std::min(schedule.max_rate_hz, dev->getParams().max_rate_hz);
    }

    auto request_fn = [this](std::chrono::milliseconds timeout) {
      return device::VL53L8CX_Device::requestMeasurementAsync(_vl53l8cx_devices, timeout);
    };
    auto fetch_fn = [this](std::chrono::milliseconds timeout, std::vector<std::future<bool>>& out_futures) {
      for (auto* dev : _vl53l8cx_devices) {
        out_futures.push_back(dev->fetchMeasurementAsync(timeout));
      }
    };

    _group_executors.push_back(std::make_unique<FunctionMeasurementGroupExecutor>(schedule, PublishTarget::Depth, true, std::move(request_fn), std::move(fetch_fn)));
  }

  if (!_tmf8829_devices.empty()) {
    SensorGroupSchedule schedule;
    schedule.type        = device::DeviceType::TMF8829;
    schedule.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _tmf8829_devices) {
      schedule.max_rate_hz = std::min(schedule.max_rate_hz, dev->getParams().max_rate_hz);
    }

    auto request_fn = [this](std::chrono::milliseconds timeout) {
      return device::TMF8829_Device::requestMeasurementAsync(_tmf8829_devices, timeout);
    };
    auto fetch_fn = [this](std::chrono::milliseconds timeout, std::vector<std::future<bool>>& out_futures) {
      for (auto* dev : _tmf8829_devices) {
        out_futures.push_back(dev->fetchMeasurementAsync(timeout));
      }
    };

    _group_executors.push_back(std::make_unique<FunctionMeasurementGroupExecutor>(schedule, PublishTarget::Depth, true, std::move(request_fn), std::move(fetch_fn)));
  }

  if (!_htpa32_devices.empty()) {
    SensorGroupSchedule schedule;
    schedule.type        = device::DeviceType::HTPA32;
    schedule.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _htpa32_devices) {
      schedule.max_rate_hz = std::min(schedule.max_rate_hz, dev->getParams().max_rate_hz);
    }

    auto request_fn = [this](std::chrono::milliseconds timeout) {
      return device::HTPA32_Device::requestMeasurementAsync(_htpa32_devices, timeout);
    };
    auto fetch_fn = [this](std::chrono::milliseconds timeout, std::vector<std::future<bool>>& out_futures) {
      for (auto* dev : _htpa32_devices) {
        out_futures.push_back(dev->fetchMeasurementAsync(timeout));
      }
    };

    _group_executors.push_back(std::make_unique<FunctionMeasurementGroupExecutor>(schedule, PublishTarget::Thermal, false, std::move(request_fn), std::move(fetch_fn)));
  }

  if (!_lights.empty()) {
    SensorGroupSchedule schedule;
    schedule.type        = device::DeviceType::WS2812b;
    schedule.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _lights) {
      schedule.max_rate_hz = std::min(schedule.max_rate_hz, dev->getParams().max_rate_hz);
    }

    _group_executors.push_back(std::make_unique<FunctionMeasurementGroupExecutor>(schedule, PublishTarget::None, false, nullptr, nullptr));
  }

  std::vector<SensorGroupSchedule> schedule;
  schedule.reserve(_group_executors.size());
  for (const auto& group : _group_executors) {
    schedule.push_back(group->schedule());
  }

  _base_rate_hz = computeScheduleFastest(schedule);
  //_base_rate_hz = computeScheduleCommonMultiple(schedule);

  for (std::size_t i = 0; i < _group_executors.size(); ++i) {
    _group_executors[i]->schedule().divisor           = schedule[i].divisor;
    _group_executors[i]->schedule().effective_rate_hz = schedule[i].effective_rate_hz;
  }

  if (_base_rate_hz > 0.0) {
    _tick_period = std::chrono::duration<double>(1.0 / _base_rate_hz);
  } else {
    _base_rate_hz = FALLBACK_LOOP_RATE_HZ;
    _tick_period  = std::chrono::duration<double>(1.0 / FALLBACK_LOOP_RATE_HZ);
  }

  logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Scheduler: base rate = " + std::to_string(_base_rate_hz) + " Hz, tick period = " + std::to_string(_tick_period.count() * 1000.0) + " ms");
  for (const auto& group : _group_executors) {
    const auto& g = group->schedule();
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "  Group " + device::toString(g.type) + ": divisor = " + std::to_string(g.divisor) + ", effective rate = " + std::to_string(g.effective_rate_hz) + " Hz");
  }
}

bool MeasurementManagerImpl::isGroupDue(const MeasurementGroupExecutor& group) const {
  return (_tick_count % group.schedule().divisor) == 0;
}

/* =======================================================================================
        Subscriptions
==========================================================================================
*/

subscription::Subscription MeasurementManagerImpl::subscribeToStateChanges(std::function<void(const ManagerState state)> callback) {
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

void MeasurementManagerImpl::notifyState(const ManagerState state) {
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
        // Phase is waiting for I/O or a timer — yield to avoid busy-spinning.
        std::this_thread::yield();
      }
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in scheduler: " + std::string(e.what()));
      _phase = Phase::error_comm_enter;
    }
  }
}

void MeasurementManagerImpl::clearScheduleFlags() {
  for (auto& group : _group_executors) {
    group->clearCycleState();
  }
}

void MeasurementManagerImpl::resetTickTiming() {
  _tick_count     = 0;
  _next_tick_time = std::chrono::steady_clock::now();
}

void MeasurementManagerImpl::prepareTickWaitPending() {
  _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
  _phase          = Phase::tick_wait_pending;
}

void MeasurementManagerImpl::resumeMeasurementLoop(bool notify_running) {
  resetTickTiming();
  clearScheduleFlags();
  if (notify_running) {
    notifyState(ManagerState::Running);
  }
  prepareTickWaitPending();
}

MeasurementManagerImpl::AsyncPollResult MeasurementManagerImpl::pollPendingRequests() {
  for (auto& group : _group_executors) {
    if (!group->schedule().has_pending_request) {
      continue;
    }

    const auto result = group->pollRequestReady(_phase_deadline);
    if (result == AsyncPollResult::Waiting || result == AsyncPollResult::TimedOut || result == AsyncPollResult::Failed) {
      return result;
    }

    group->schedule().has_fetch_ready     = true;
    group->schedule().has_pending_request = false;
  }

  return AsyncPollResult::Ready;
}

MeasurementManagerImpl::AsyncPollResult MeasurementManagerImpl::pollFetchReady() {
  for (auto& group : _group_executors) {
    const auto result = group->pollFetchReady(_phase_deadline);
    if (result == AsyncPollResult::Waiting || result == AsyncPollResult::TimedOut || result == AsyncPollResult::Failed) {
      return result;
    }
  }

  return AsyncPollResult::Ready;
}

bool MeasurementManagerImpl::collectAndPublishFetchedMeasurements() {
  bool success = true;

  bool depth_publish_needed   = false;
  bool thermal_publish_needed = false;
  for (auto& group : _group_executors) {
    if (group->hasFetchWork()) {
      if (group->getPublishTarget() == PublishTarget::Depth) {
        depth_publish_needed = true;
      } else if (group->getPublishTarget() == PublishTarget::Thermal) {
        thermal_publish_needed = true;
      }
    }

    success &= group->collectFetchResults();
  }

  if (depth_publish_needed) {
    publishDepthMeasurements();
  }
  if (thermal_publish_needed) {
    publishThermalMeasurements();
  }

  for (auto& group : _group_executors) {
    group->schedule().has_fetch_ready = false;
  }

  return success;
}

bool MeasurementManagerImpl::handleInitializationPhase() {
  switch (_phase) {
  case Phase::init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManager scheduler");
    _phase = Phase::reset_sensors;
    return true;
  }

  case Phase::reset_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
    board::resetBoards();
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
    resumeMeasurementLoop(true);
    return true;
  }

  default:
    return false;
  }
}

bool MeasurementManagerImpl::handleTickPhase() {
  switch (_phase) {
  case Phase::tick_wait_pending: {
    const auto pending_result = pollPendingRequests();
    if (pending_result == AsyncPollResult::Waiting) {
      return false;
    }
    if (pending_result == AsyncPollResult::TimedOut) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout waiting for pending data-available signal.");
      _phase = Phase::error_meas_enter;
      return false;
    }
    if (pending_result == AsyncPollResult::Failed) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Data-available signal reported failure.");
      _phase = Phase::error_meas_enter;
      return false;
    }

    // All pending groups are ready — advance.
    _phase = Phase::tick_request;
    return true;
  }

  case Phase::tick_request: {
    // Fire new measurement requests for groups due this tick so sensor hardware
    // starts its next cycle while we are transferring the previous cycle's data.
    requestMeasurements();

    // Launch fetch operations for groups that were promoted to fetch-ready above.
    launchFetchFutures();

    _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
    _phase          = Phase::tick_fetch_wait;
    return true;
  }

  case Phase::tick_fetch_wait: {
    const auto fetch_result = pollFetchReady();
    if (fetch_result == AsyncPollResult::Waiting) {
      return false;
    }
    if (fetch_result == AsyncPollResult::TimedOut) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout fetching measurement data.");
      clearScheduleFlags();
      _phase = Phase::error_meas_enter;
      return false;
    }
    if (fetch_result == AsyncPollResult::Failed) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Fetching measurement data failed while polling futures.");
      _phase = Phase::error_meas_enter;
      return true;
    }

    const bool success = collectAndPublishFetchedMeasurements();
    if (!success) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Fetching measurement data failed.");
      _phase = Phase::error_meas_enter;
      return true;
    }

    _phase = Phase::tick_actions;
    return true;
  }

  case Phase::tick_actions: {
    executeDeviceActions();

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
    prepareTickWaitPending();
    return true;
  }

  default:
    return false;
  }
}

bool MeasurementManagerImpl::handleMeasurementErrorPhase() {
  switch (_phase) {
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
    clearScheduleFlags();

    _tick_count = 0;
    requestMeasurements();
    _error_attempts++;

    _phase_deadline = std::chrono::steady_clock::now() + _params.timeout;
    _phase          = Phase::error_meas_wait;
    return true;
  }

  case Phase::error_meas_wait: {
    const auto pending_result = pollPendingRequests();
    if (pending_result == AsyncPollResult::Waiting) {
      return false;
    }
    if (pending_result == AsyncPollResult::TimedOut || pending_result == AsyncPollResult::Failed) {
      _phase = Phase::error_meas_retry;
      return false;
    }

    // All pending futures resolved — restart succeeded.
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Measurement restart succeeded after " + std::to_string(_error_attempts) + " attempt(s).");
    resumeMeasurementLoop(true);
    return true;
  }

  default:
    return false;
  }
}

bool MeasurementManagerImpl::handleCommunicationErrorPhase() {
  switch (_phase) {
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
      resumeMeasurementLoop(true);
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
      resumeMeasurementLoop(true);
    } else {
      _phase = Phase::error_comm_repair;
    }
    return true;
  }

  default:
    return false;
  }
}

bool MeasurementManagerImpl::handleShutdownPhase() {
  if (_phase != Phase::shutdown) {
    return false;
  }

  logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Shutting down scheduler.");
  notifyState(ManagerState::Shutdown);
  _is_running = false;
  return true;
}

bool MeasurementManagerImpl::runPhase() {
  if (handleInitializationPhase()) {
    return true;
  }
  if (handleTickPhase()) {
    return true;
  }
  if (handleMeasurementErrorPhase()) {
    return true;
  }
  if (handleCommunicationErrorPhase()) {
    return true;
  }
  if (handleShutdownPhase()) {
    return true;
  }

  return true;
}

/* =======================================================================================
        Tick sub-steps
==========================================================================================
*/

void MeasurementManagerImpl::launchFetchFutures() {
  // Launch async fetch for every group that was promoted to fetch-ready.
  // Futures are stored and polled non-blocking in tick_fetch_wait.
  for (auto& group : _group_executors) {
    if (!group->schedule().has_fetch_ready) {
      continue;
    }

    group->launchFetch(_params.timeout);
  }
}

void MeasurementManagerImpl::requestMeasurements() {
  for (auto& group : _group_executors) {
    if (!isGroupDue(*group)) {
      continue;
    }

    if (group->requestMeasurement(_params.timeout)) {
      group->schedule().has_pending_request = true;
      group->schedule().has_fetch_ready     = false;
    }
  }
}

void MeasurementManagerImpl::executeDeviceActions() {
  for (auto* dev : _sensor_ring->getDevices()) {
    for (auto& action : dev->drainActions()) {
      try {
        action();
      } catch (const std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception in device action: " + std::string(e.what()));
      }
    }
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
