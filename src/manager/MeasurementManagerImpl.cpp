#include "manager/MeasurementManagerImpl.hpp"

#include "device/SensorBoardCommands.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/SensorBoard.hpp"
#include "sensorring/SensorBus.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/IDevice.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace std::chrono_literals;

namespace eduart {

namespace sensorring {

namespace manager {

MeasurementManagerImpl::MeasurementManagerImpl(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring)
    : _params(params)
    , _manager_state(ManagerState::Uninitialized)
    , _phase(Phase::init)
    , _sensor_ring(std::move(sensor_ring))
    , _base_rate_hz(0.0)
    , _tick_period(0.0)
    , _next_tick_time(std::chrono::steady_clock::now())
    , _tick_count(0)
    , _is_running(false) {

  if (_sensor_ring->getDevices().empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Empty SensorRing passed to MeasurementManager");
    return;
  }

  if (_params.timeout == 0ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter is 0.0s");
  } else if (_params.timeout < 200ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter of " + std::to_string(_params.timeout.count()) + " ms is probably too low");
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
  _schedule.clear();

  // Build one group per device type that has enabled sensors.
  if (!_vl53l8cx_devices.empty()) {
    SensorGroupSchedule group;
    group.type = device::DeviceType::VL53L8CX;
    // Group max rate is limited by the slowest sensor in the group.
    group.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _vl53l8cx_devices) {
      if (dev->getEnable()) {
        group.max_rate_hz = std::min(group.max_rate_hz, dev->getParams().max_rate_hz);
      }
    }
    if (group.max_rate_hz == std::numeric_limits<double>::max()) {
      group.max_rate_hz = 15.0; // Fallback default.
    }
    _schedule.push_back(group);
  }

  if (!_htpa32_devices.empty()) {
    SensorGroupSchedule group;
    group.type        = device::DeviceType::HTPA32;
    group.max_rate_hz = std::numeric_limits<double>::max();
    for (auto* dev : _htpa32_devices) {
      if (dev->getEnable()) {
        group.max_rate_hz = std::min(group.max_rate_hz, dev->getParams().max_rate_hz);
      }
    }
    if (group.max_rate_hz == std::numeric_limits<double>::max()) {
      group.max_rate_hz = 6.0; // Fallback default.
    }
    _schedule.push_back(group);
  }

  _base_rate_hz = computeScheduleFastest(_schedule, _params.frequency_tof_hz, _params.frequency_thermal_hz);
  //_base_rate_hz = computeScheduleCommonMultiple(_schedule, _params.frequency_tof_hz, _params.frequency_thermal_hz);

  if (_base_rate_hz > 0.0) {
    _tick_period = std::chrono::duration<double>(1.0 / _base_rate_hz);
  } else {
    _tick_period = std::chrono::duration<double>(0.1); // 10 Hz fallback.
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
  bool success = false;

  if (!_is_running) {
    notifyState(ManagerState::Running);
    try {
      runPhase();
      success = true;
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in scheduler: " + std::string(e.what()));
      _phase = Phase::error_handler_communication;
    }
  }

  return success;
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
      runPhase();
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in scheduler: " + std::string(e.what()));
      _phase = Phase::error_handler_communication;
    }
  }
}

void MeasurementManagerImpl::runPhase() {
  bool success = true;

  switch (_phase) {
    /* =============================================
      Initialization sequence
    ============================================= */

  case Phase::init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManager scheduler");
    _phase = Phase::reset_sensors;
    break;
  }

  case Phase::reset_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
    device::resetBoards();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    _phase = Phase::sync_lights;
    break;
  }

  case Phase::sync_lights: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Syncing all lights and set to mode pulsation");
    device::WS2812b_Device::syncLight();
    device::WS2812b_Device::setLight(device::LightMode::Pulsation, 0, 0, 0);
    _phase = Phase::get_eeprom;
    break;
  }

  case Phase::get_eeprom: {
    if (!_htpa32_devices.empty()) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Reading EEPROM from thermal sensors");
      const auto timeout_ms = _params.timeout;
      for (auto* device : _htpa32_devices) {
        if (device->getEnable()) {
          auto fut = device->getEepromAsync(timeout_ms);
          success &= fut.get();
        }
      }
    }

    if (success) {
      _phase = Phase::pre_loop_init;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to read EEPROM values from at least one sensor. Check configuration and restart.");
      _phase = Phase::shutdown;
    }
    break;
  }

  case Phase::pre_loop_init: {
    _sensor_ring->setBitRateSwitching(_params.enable_brs);
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Starting tick-based measurement loop.");
    _tick_count     = 0;
    _next_tick_time = std::chrono::steady_clock::now();

    // Clear pending flags.
    for (auto& g : _schedule) {
      g.has_pending_request = false;
      g.has_fetch_ready     = false;
    }

    _phase = Phase::tick;
    break;
  }

    /* =============================================
      Tick-based measurement loop
    ============================================= */

  case Phase::tick: {
    // Step 1: Wait for pending data from previous requests (self-regulating).
    if (!waitForPendingData()) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout while waiting for pending measurement data.");
      _phase = Phase::error_handler_measurement;
      break;
    }

    // Step 2: Request new measurements for groups due this tick.
    // Done before fetching so sensors start working on N+1 while we transfer N.
    requestMeasurements();

    // Step 3: Fetch data from groups that completed (has_fetch_ready was set before request).
    if (!fetchPendingData()) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout while fetching measurement data.");
      _phase = Phase::error_handler_measurement;
      break;
    }

    // Step 4: Execute device actions (actuators) at end of tick.
    executeDeviceActions();

    // Step 5: Sleep until next tick boundary.
    _tick_count++;
    _next_tick_time += std::chrono::duration_cast<std::chrono::steady_clock::duration>(_tick_period);
    std::this_thread::sleep_until(_next_tick_time);

    break;
  }

    /* =============================================
      Error handlers
    ============================================= */

  case Phase::error_handler_measurement: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error handler for measurement errors called.");
    notifyState(ManagerState::Error);

    if (_params.repair_errors) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Trying to restart measurements.");

      // Reset sensor state.
      for (device::IDevice* d : _sensor_ring->getDevices()) {
        if (auto bd = dynamic_cast<device::BaseDevice*>(d))
          bd->resetSensorState();
      }

      // Clear all pending flags and attempt a single request cycle.
      for (auto& g : _schedule) {
        g.has_pending_request = false;
        g.has_fetch_ready     = false;
      }

      unsigned int attempts = 0;
      success               = false;

      do {
        attempts++;
        if (!_vl53l8cx_devices.empty()) {
          auto fut = device::VL53L8CX_Device::requestMeasurementAsync(_vl53l8cx_devices, _params.timeout);
          if (fut.wait_for(_params.timeout) != std::future_status::ready) {
            success = false;
          } else {
            success = fut.get();
          }
        } else {
          success = true;
        }
      } while (!success && _is_running && (attempts < 10));

      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting measurements succeeded after " + std::to_string(attempts) + " attempts.");
        _tick_count     = 0;
        _next_tick_time = std::chrono::steady_clock::now();
        _phase          = Phase::tick;
        notifyState(ManagerState::Running);
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to restart measurements. Resetting all sensors.");
        _phase = Phase::reset_sensors;
      }
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Will not attempt to restart measurements because parameter \"repair_errors\" is set to \"false\".");
      _phase = Phase::shutdown;
    }
    break;
  }

  case Phase::error_handler_communication: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error handler for communication errors called.");
    notifyState(ManagerState::Error);

    if (_params.repair_errors) {
      bool communication_error = false;
      for (auto& bus : _sensor_ring->getSensorBuses()) {
        auto interface = bus->getInterface();
        communication_error |= interface->hasError();
      }

      if (communication_error) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Communication error detected. Trying to restart affected interfaces.");

        unsigned int attempts = 0;
        success               = true;

        do {
          attempts++;
          success = true;
          for (auto& bus : _sensor_ring->getSensorBuses()) {
            auto interface = bus->getInterface();
            if (interface->hasError()) {
              try {
                success &= interface->repairInterface();
              } catch (std::runtime_error&) {
                success = false;
              }
            }
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(250));
        } while (!success && _is_running && (attempts < 40));
      }

      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting communication succeeded.");
        _tick_count     = 0;
        _next_tick_time = std::chrono::steady_clock::now();
        for (auto& g : _schedule) {
          g.has_pending_request = false;
          g.has_fetch_ready     = false;
        }
        _phase = Phase::tick;
        notifyState(ManagerState::Running);
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to restart communication. Please check the interfaces.");
        _phase = Phase::shutdown;
      }
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Will not attempt to restart communication because parameter \"repair_errors\" is set to \"false\".");
      _phase = Phase::shutdown;
    }
    break;
  }

    /* =============================================
      Shutdown
    ============================================= */

  case Phase::shutdown: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Shutting down scheduler.");
    notifyState(ManagerState::Shutdown);
    _is_running = false;
    break;
  }
  };
}

/* =======================================================================================
        Tick sub-steps
==========================================================================================
*/

bool MeasurementManagerImpl::waitForPendingData() {
  // Wait for all groups that have a pending request from the previous tick.
  // After waiting, promote them to fetch-ready state.
  for (auto& group : _schedule) {
    if (!group.has_pending_request) {
      continue;
    }

    if (group.type == device::DeviceType::VL53L8CX) {
      // Wait for the data-available future that was launched in the previous tick's
      // requestMeasurements(). This blocks until all sensors signal measurement complete.
      if (_tof_data_available_future.valid()) {
        if (_tof_data_available_future.wait_for(_params.timeout) != std::future_status::ready || !_tof_data_available_future.get()) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "VL53L8CX data-available wait timed out.");
          return false;
        }
      }
    }

    if (group.type == device::DeviceType::HTPA32) {
      // HTPA32 request is fire-and-forget. The actual wait happens during fetch
      // (fetchMeasurementAsync blocks until data arrives).
    }

    // Data is available — mark ready for fetch.
    group.has_fetch_ready     = true;
    group.has_pending_request = false;
  }

  return true;
}

bool MeasurementManagerImpl::fetchPendingData() {
  for (auto& group : _schedule) {
    if (!group.has_fetch_ready) {
      continue;
    }

    if (group.type == device::DeviceType::VL53L8CX) {
      for (auto* dev : _vl53l8cx_devices) {
        if (!dev->getEnable())
          continue;
        auto fut = dev->fetchMeasurementAsync(_params.timeout);
        if (fut.wait_for(_params.timeout) != std::future_status::ready || !fut.get()) {
          return false;
        }
      }
      publishDepthMeasurements();
    }

    if (group.type == device::DeviceType::HTPA32) {
      for (auto* dev : _htpa32_devices) {
        if (!dev->getEnable())
          continue;
        auto fut = dev->fetchMeasurementAsync(_params.timeout);
        if (fut.wait_for(_params.timeout) != std::future_status::ready || !fut.get()) {
          return false;
        }
      }
      publishThermalMeasurements();
    }

    group.has_fetch_ready = false;
  }

  return true;
}

void MeasurementManagerImpl::requestMeasurements() {
  for (auto& group : _schedule) {
    if (!isGroupDue(group)) {
      continue;
    }

    if (group.type == device::DeviceType::VL53L8CX) {
      if (!_vl53l8cx_devices.empty()) {
        // Launch request — the async thread sends the broadcast and waits for
        // data-available. The future is consumed in the next tick's waitForPendingData().
        _tof_data_available_future = device::VL53L8CX_Device::requestMeasurementAsync(_vl53l8cx_devices, _params.timeout);
        group.has_pending_request  = true;
      }
    }

    if (group.type == device::DeviceType::HTPA32) {
      if (!_htpa32_devices.empty()) {
        // Fire-and-forget broadcast request.
        device::HTPA32_Device::requestMeasurementAsync(_htpa32_devices, _params.timeout);
        group.has_pending_request = true;
      }
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

} // namespace manager

} // namespace sensorring

} // namespace eduart
