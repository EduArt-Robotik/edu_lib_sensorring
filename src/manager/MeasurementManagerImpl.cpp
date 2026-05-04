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
    , _measurement_state(MeasurementState::init)
    , _sensor_ring(std::move(sensor_ring))
    , _first_measurement(true)
    , _tof_measurement_period(1.0F / params.frequency_tof_hz)
    , _thermal_measurement_period(1.0F / params.frequency_thermal_hz)
    , _last_tof_measurement_timestamp(std::chrono::steady_clock::now())
    , _last_thermal_measurement_timestamp(std::chrono::steady_clock::now())
    , _is_tof_throttled(params.frequency_tof_hz > 0.0)
    , _is_thermal_throttled(params.frequency_thermal_hz > 0.0)
    , _thermal_measurement_flag(false)
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

  // Populate typed device vectors via dynamic_cast
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

  _manager_state = ManagerState::Initialized;
}

MeasurementManagerImpl::~MeasurementManagerImpl() noexcept {
  stopMeasuring();
}

ManagerParams MeasurementManagerImpl::getParams() const noexcept {
  return _params;
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
      StateMachine();
      success = true;
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in state machine: " + std::string(e.what()));
      _measurement_state = MeasurementState::error_handler_communication;
    }
  }

  return success;
}

bool MeasurementManagerImpl::startMeasuring() noexcept {
  if (!_is_running) {
    _is_running    = true;
    _worker_thread = std::thread(&MeasurementManagerImpl::StateMachineWorker, this);
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
  State machine function
==========================================================================================
*/
void MeasurementManagerImpl::StateMachineWorker() noexcept {
  while (_is_running) {
    try {
      StateMachine();
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in state machine: " + std::string(e.what()));
      _measurement_state = MeasurementState::error_handler_communication;
    }
  }
}

void MeasurementManagerImpl::StateMachine() {
  bool success               = true;
  const bool tof_enabled     = !_vl53l8cx_devices.empty();
  const bool thermal_enabled = !_htpa32_devices.empty();

  switch (_measurement_state) {
    /* =============================================
      Initialization part of the state machine
      Runs once at start and may be triggered
      again on error conditions
    ============================================= */

  case MeasurementState::init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManager state machine");
    _measurement_state = MeasurementState::reset_sensors;
    break;
  }

  case MeasurementState::reset_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
    device::resetBoards();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    _measurement_state = MeasurementState::sync_lights;
    break;
  }

  case MeasurementState::sync_lights: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Syncing all lights and set to mode pulsation");
    device::WS2812b_Device::syncLight();
    device::WS2812b_Device::setLight(device::LightMode::Pulsation, 0, 0, 0);
    _measurement_state = MeasurementState::get_eeprom;
    break;
  }

  case MeasurementState::get_eeprom: {
    if (thermal_enabled) {
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
      _measurement_state = MeasurementState::pre_loop_init;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to read EEPROM values from at least one sensor. Check configuration and restart.");
      _measurement_state = MeasurementState::shutdown;
    }
    break;
  }

  case MeasurementState::pre_loop_init: {
    _sensor_ring->setBitRateSwitching(_params.enable_brs);
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Starting to fetch measurements now.");
    _last_tof_measurement_timestamp     = std::chrono::steady_clock::now();
    _last_thermal_measurement_timestamp = std::chrono::steady_clock::now();
    _measurement_state                  = MeasurementState::device_actions;
    break;
  }

    /* =============================================
      Loop part of the state machine
      Runs continuously to fetch data
    ============================================= */

  case MeasurementState::device_actions: {
    // Drain and execute all per-device action queues.
    for (auto* dev : _sensor_ring->getDevices()) {
      for (auto& action : dev->drainActions()) {
        try {
          action();
        } catch (const std::exception& e) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception in device action: " + std::string(e.what()));
        }
      }
    }
    _measurement_state = MeasurementState::request_tof_measurement;
    break;
  }

  case MeasurementState::request_tof_measurement: {
    if (tof_enabled) {
      _tof_request_future = device::VL53L8CX_Device::requestMeasurementAsync(_vl53l8cx_devices, _params.timeout);
    }
    _last_tof_measurement_timestamp = std::chrono::steady_clock::now();
    _measurement_state              = MeasurementState::request_thermal_measurement;
    break;
  }

  case MeasurementState::request_thermal_measurement: {
    if (thermal_enabled && !_thermal_measurement_flag) {
      if (!_is_thermal_throttled || ((std::chrono::steady_clock::now() - _last_thermal_measurement_timestamp) > _thermal_measurement_period)) {
        device::HTPA32_Device::requestMeasurementAsync(_htpa32_devices, _params.timeout);
        _last_thermal_measurement_timestamp = std::chrono::steady_clock::now();
        _thermal_measurement_flag           = true;
      }
    }
    _measurement_state = MeasurementState::wait_for_data;
    break;
  }

  case MeasurementState::wait_for_data: {
    if (tof_enabled && _tof_request_future.valid()) {
      if (_tof_request_future.wait_for(_params.timeout) != std::future_status::ready) {
        success = false;
      } else {
        success = _tof_request_future.get();
      }
    }

    if (success) {
      if (_first_measurement) {
        _first_measurement = false;
        _measurement_state = MeasurementState::device_actions;
        break;
      } else {
        _measurement_state = MeasurementState::fetch_tof_data;
      }
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occurred while waiting for completion of measurements.");
      _measurement_state = MeasurementState::error_handler_measurement;
    }
    break;
  }

  case MeasurementState::fetch_tof_data: {
    if (tof_enabled) {
      for (auto* dev : _vl53l8cx_devices) {
        if (!dev->getEnable())
          continue;
        auto fut = dev->fetchMeasurementAsync(_params.timeout);
        if (fut.wait_for(_params.timeout) != std::future_status::ready || !fut.get()) {
          success = false;
          break;
        }
      }

      if (success) {
        publishDepthMeasurements();
      }
    }

    if (success) {
      _measurement_state = MeasurementState::fetch_thermal_data;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occurred while fetching tof measurements.");
      _measurement_state = MeasurementState::error_handler_measurement;
    }
    break;
  }

  case MeasurementState::fetch_thermal_data: {
    if (thermal_enabled && _thermal_measurement_flag) {
      for (auto* dev : _htpa32_devices) {
        if (!dev->getEnable())
          continue;
        auto fut = dev->fetchMeasurementAsync(_params.timeout);
        if (fut.wait_for(_params.timeout) != std::future_status::ready || !fut.get()) {
          success = false;
          break;
        }
      }

      if (success) {
        publishThermalMeasurements();
      }
      _thermal_measurement_flag = false;
    }

    if (success) {
      _measurement_state = MeasurementState::throttle_measurement;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occurred while fetching thermal measurements.");
      _measurement_state = MeasurementState::error_handler_measurement;
    }
    break;
  }

  case MeasurementState::throttle_measurement: {
    if (tof_enabled && _is_tof_throttled) {
      std::this_thread::sleep_until(_last_tof_measurement_timestamp + _tof_measurement_period);
    }
    _measurement_state = MeasurementState::device_actions;
    break;
  }

    /* =============================================
      Error handler
    ============================================= */

  case MeasurementState::error_handler_measurement: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error handler for measurement errors called.");
    notifyState(ManagerState::Error);

    unsigned int attempts = 0;

    if (_params.repair_errors) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Trying to restart measurements.");

      if (success) {
        attempts = 0;
        for (device::IDevice* d : _sensor_ring->getDevices())
          if (auto bd = dynamic_cast<device::BaseDevice*>(d))
            bd->resetSensorState();
        do {
          attempts++;
          if (!_vl53l8cx_devices.empty()) {
            _tof_request_future = device::VL53L8CX_Device::requestMeasurementAsync(_vl53l8cx_devices, _params.timeout);
            if (_tof_request_future.wait_for(_params.timeout) != std::future_status::ready) {
              success = false;
            } else {
              success = _tof_request_future.get();
            }
          }
        } while (!success && _is_running && (attempts < 10));
      }
    }

    if (_params.repair_errors) {
      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting measurements succeeded after " + std::to_string(attempts) + " attempts.");
        _measurement_state = MeasurementState::device_actions;
        notifyState(ManagerState::Running);
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to restart measurements. Resetting all sensors.");
        _measurement_state = MeasurementState::reset_sensors;
      }
    } else {
      logger::Logger::getInstance()->log(
          logger::LogVerbosity::Info, "Will not attempt to restart measurements because parameter "
                                      "\"repair_errors\" is set to \"false\".");
      _measurement_state = MeasurementState::shutdown;
    }
    break;
  }

  case MeasurementState::error_handler_communication: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error handler for communication errors called.");
    notifyState(ManagerState::Error);

    unsigned int attempts = 0;

    if (_params.repair_errors) {
      bool communication_error = false;
      for (auto& bus : _sensor_ring->getSensorBuses()) {
        auto interface = bus->getInterface();
        communication_error |= interface->hasError();
      }

      if (communication_error) {
        logger::Logger::getInstance()->log(
            logger::LogVerbosity::Info, "Communication error detected. Trying "
                                        "to restart affected interfaces.");

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
    }

    if (_params.repair_errors) {
      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting communication succeeded after " + std::to_string(attempts) + " attempts.");
        _measurement_state = MeasurementState::device_actions;
        notifyState(ManagerState::Running);
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to restart communication. Please check the interfaces.");
        _measurement_state = MeasurementState::shutdown;
      }
    } else {
      logger::Logger::getInstance()->log(
          logger::LogVerbosity::Info, "Will not attempt to restart measurements because parameter "
                                      "\"repair_errors\" is set to \"false\".");
      _measurement_state = MeasurementState::shutdown;
    }
    break;
  }

    /* =============================================
      Shutdown
    ============================================= */

  case MeasurementState::shutdown: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Shutting down state machine.");
    notifyState(ManagerState::Shutdown);
    _is_running = false;
    break;
  }
  };
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
