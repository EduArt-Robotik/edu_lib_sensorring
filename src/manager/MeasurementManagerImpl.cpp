#include "manager/MeasurementManagerImpl.hpp"

#include "sensorring/SensorBoard.hpp"
#include "sensorring/SensorBus.hpp"
#include "sensorring/device/IDevice.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace std::chrono_literals;

namespace eduart {

namespace manager {

MeasurementManagerImpl::MeasurementManagerImpl(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring)
    : _params(params)
    , _manager_state(ManagerState::Uninitialized)
    , _measurement_state(MeasurementState::init)
    , _sensor_ring(std::move(sensor_ring))
    , _tof_enabled(false)
    , _thermal_enabled(false)
    , _first_measurement(true)
    , _tof_measurement_period(1.0F / params.frequency_tof_hz)
    , _thermal_measurement_period(1.0F / params.frequency_thermal_hz)
    , _last_tof_measurement_timestamp(std::chrono::steady_clock::now())
    , _last_thermal_measurement_timestamp(std::chrono::steady_clock::now())
    , _is_tof_throttled(params.frequency_tof_hz > 0.0)
    , _is_thermal_throttled(params.frequency_thermal_hz > 0.0)
    , _thermal_measurement_flag(false)
    , _is_running(false)
    , _device_groups(
          {
              { DeviceGroup::VL53L8CX, device::DeviceGroup::createFromDevicesOfType<device::VL53L8CX_Device>(_sensor_ring->getDevices()) },
              { DeviceGroup::HTPA32,   device::DeviceGroup::createFromDevicesOfType<device::HTPA32_Device>(_sensor_ring->getDevices())   },
              { DeviceGroup::WS2812B,  device::DeviceGroup::createFromDevicesOfType<device::WS2812b_Device>(_sensor_ring->getDevices())  },
}) {
  if (_params.timeout == 0ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter is 0.0s");
  } else if (_params.timeout < 200ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter of " + std::to_string(_params.timeout.count()) + " ms is probably too low");
  }

  // check if there are active tof or thermal sensors (device-group based)
  _device_groups.at(DeviceGroup::VL53L8CX).invokeForEachDevice([this](device::IDevice* device) {
    if (dynamic_cast<device::VL53L8CX_Device*>(device)->getEnable())
      _tof_enabled = true;
  });
  _device_groups.at(DeviceGroup::HTPA32).invokeForEachDevice([this](device::IDevice* device) {
    if (dynamic_cast<device::HTPA32_Device*>(device)->getEnable())
      _thermal_enabled = true;
  });
  // prepare state machine
  _manager_state = ManagerState::Initialized;
}

MeasurementManagerImpl::~MeasurementManagerImpl() noexcept {
  stopMeasuring();
}

ManagerParams MeasurementManagerImpl::getParams() const noexcept {
  return _params;
}

ring::SensorRing* MeasurementManagerImpl::getSensorRing() const noexcept {
  return _sensor_ring.get();
}

void MeasurementManagerImpl::enqueueExtraAction(std::function<void()> action) {
  if (!action) {
    return;
  }
  std::lock_guard<std::mutex> lock(_extra_actions_mutex);
  _extra_actions.emplace(std::move(action));
}

/* =======================================================================================
        Handle clients
==========================================================================================
*/

SubscriberToken MeasurementManagerImpl::subscribeToStateChanges(std::function<void(const ManagerState state)> callback) {
  if (!callback) {
    return SubscriberToken();
  }
  auto token = SubscriberToken::getNextToken();
  LockGuard lock(_subscriber_mutex);
  _state_subscriptions.emplace(token, std::move(callback));
  return token;
}

SubscriberToken MeasurementManagerImpl::subscribeToDeviceGroup(DeviceGroup key, std::function<void(const device::DeviceGroup&)> callback) {
  if (!callback) {
    return SubscriberToken();
  }
  auto token = SubscriberToken::getNextToken();
  LockGuard lock(_subscriber_mutex);
  auto& key_subs = _device_subscriptions.try_emplace(key).first->second;
  key_subs.emplace(token, std::move(callback));
  return token;
}

void MeasurementManagerImpl::unsubscribe(SubscriberToken token) {
  if (!token.isValid()) {
    return;
  }
  LockGuard lock(_subscriber_mutex);
  _state_subscriptions.erase(token);
  for (auto& [key, subscriptions] : _device_subscriptions) {
    subscriptions.erase(token);
  }
}

bool MeasurementManagerImpl::waitForMeasurementFuture(MeasurementFutureKey key, std::chrono::steady_clock::duration timeout) noexcept {
  auto it = _measurement_futures.find(key);
  if (it == _measurement_futures.end()) {
    // Nothing to wait for for this key.
    return true;
  }
  auto& fut = it->second;
  if (fut.wait_for(timeout) != std::future_status::ready) {
    return false;
  }
  return fut.get();
}

int MeasurementManagerImpl::notifyToFData() {
  int error_frames = 0;

  _device_groups.at(DeviceGroup::VL53L8CX).invokeForEachDeviceOfType<device::VL53L8CX_Device>([&error_frames](device::VL53L8CX_Device* device) {
    if (!device->getEnable())
      return;
    auto state = device->getLatestRawMeasurement().second;
    if (state != device::SensorState::SensorOK) {
      error_frames++;
    }
  });

  {
    LockGuard lock(_subscriber_mutex);
    const device::DeviceGroup& tof_group = _device_groups.at(DeviceGroup::VL53L8CX);
    auto it                              = _device_subscriptions.find(DeviceGroup::VL53L8CX);
    if (it != _device_subscriptions.end()) {
      for (auto& sub : it->second) {
        if (sub.second) {
          try {
            sub.second(tof_group);
          } catch (const std::exception& e) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Device group VL53L8CX subscription callback threw: " + std::string(e.what()));
          } catch (...) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Device group VL53L8CX subscription callback threw unknown exception.");
          }
        }
      }
    }
  }

  return error_frames;
}

int MeasurementManagerImpl::notifyThermalData() {
  int error_frames = 0;

  _device_groups.at(DeviceGroup::HTPA32).invokeForEachDeviceOfType<device::HTPA32_Device>([&error_frames](device::HTPA32_Device* device) {
    if (!device->getEnable())
      return;
    auto state = device->getLatestMeasurement().second;
    if (state != device::SensorState::SensorOK) {
      error_frames++;
    }
  });

  {
    LockGuard lock(_subscriber_mutex);
    const device::DeviceGroup& thermal_group = _device_groups.at(DeviceGroup::HTPA32);
    auto it                                  = _device_subscriptions.find(DeviceGroup::HTPA32);
    if (it != _device_subscriptions.end()) {
      for (auto& sub : it->second) {
        if (sub.second) {
          try {
            sub.second(thermal_group);
          } catch (const std::exception& e) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Device group HTPA32 subscription callback threw: " + std::string(e.what()));
          } catch (...) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Device group HTPA32 subscription callback threw unknown exception.");
          }
        }
      }
    }
  }

  return error_frames;
}

void MeasurementManagerImpl::notifyState(const ManagerState state) {
  LockGuard lock(_subscriber_mutex);
  for (auto& sub : _state_subscriptions) {
    if (sub.second) {
      try {
        sub.second(state);
      } catch (const std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "State subscription callback threw: " + std::string(e.what()));
      } catch (...) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "State subscription callback threw unknown exception.");
      }
    }
  }
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
    if (_tof_enabled || _thermal_enabled) {
      notifyState(ManagerState::Running);
      try {
        StateMachine();
        success = true;
      } catch (const std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in state machine: " + std::string(e.what()));
        _measurement_state = MeasurementState::error_handler_communication;
      }
    }
  }

  return success;
}

bool MeasurementManagerImpl::startMeasuring() noexcept {
  if (!_is_running) {
    if (_tof_enabled || _thermal_enabled) {
      _is_running    = true;
      _worker_thread = std::thread(&MeasurementManagerImpl::StateMachineWorker, this);
      notifyState(ManagerState::Running);
      return true;
    }
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
    // no wait command here, the individual states of the state machine
    // provide natural throttling
    try {
      StateMachine();
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught exception in state machine: " + std::string(e.what()));
      _measurement_state = MeasurementState::error_handler_communication;
    }
  }
}

void MeasurementManagerImpl::StateMachine() {
  bool success = true;
  switch (_measurement_state) {
    /* =============================================
      Initialization part of the state machine
      Runs once at start and may be triggered
      again on error conditions
    ============================================= */

  case MeasurementState::init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManager state machine");

    // state transition
    _measurement_state = MeasurementState::reset_sensors;
    break;
  }

  case MeasurementState::reset_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
    device::SensorBoard::resetBoards();
    std::this_thread::sleep_for(std::chrono::seconds(2)); // sleep 2 seconds -> boards need time to init their vl53l8 sensors

    // state transition
    _measurement_state = MeasurementState::sync_lights;
    break;
  }

  case MeasurementState::sync_lights: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Syncing all lights and set to mode pulsation");

    device::WS2812b_Device::syncLight();
    device::WS2812b_Device::setLight(light::LightMode::Pulsation, 0, 0, 0);

    // state transition
    _measurement_state = MeasurementState::enumerate_sensors;
    break;
  }

  case MeasurementState::enumerate_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Enumerating all connected sensors");

    success = _sensor_ring->enumerateDevices();

    for (auto sensor_bus : _sensor_ring->getInterfaces()) {
      logger::Logger::getInstance()->log(
          logger::LogVerbosity::Info,
          "Counted " + std::to_string(sensor_bus->getEnumerationCount()) + " sensor boards on interface " + sensor_bus->getInterface()->getInterfaceName() + ", " + std::to_string(sensor_bus->getSensorCount()) + " are configured.");

      if (sensor_bus->getSensorCount() && _params.print_topology) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, _sensor_ring->printTopology());
      }

      if (sensor_bus->getEnumerationCount() > 0) {

        if (sensor_bus->getSensorCount() != sensor_bus->getEnumerationCount()) {
          if (_params.enforce_topology) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Counted the wrong number of sensors and the parameter \"enforce_topology\" is set to \"true\". Check topology and restart.");
          } else {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Counted the wrong number of sensors but the parameter \"enforce_topology\" is set to \"false\". Measurements will only include the configured sensors.");
            success = true;
          }
        }
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Counted 0 sensor boards, no work to do here. Check topology and restart.");
      }
    }

    // state transition
    if (success) {
      _measurement_state = MeasurementState::get_eeprom;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to enumerate sensors");
      _measurement_state = MeasurementState::shutdown;
    }
    break;
  }

  case MeasurementState::get_eeprom: {
    if (_thermal_enabled) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Reading EEPROM from thermal sensors");

      const auto timeout_ms = _params.timeout;
      _device_groups.at(DeviceGroup::HTPA32).invokeForEachDeviceOfType<device::HTPA32_Device>([&success, timeout_ms](device::HTPA32_Device* device) {
        auto fut = device->getEpromAsync(timeout_ms);
        if (!fut.get()) {
          success = false;
        }
      });
    }

    // state transition
    if (success) {
      _measurement_state = MeasurementState::pre_loop_init;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to read EEPROM values from at least one sensor. Check configuration and restart.");
      _measurement_state = MeasurementState::shutdown;
    }
    break;
  }

  case MeasurementState::pre_loop_init: {
    // enable bit rate switching
    _sensor_ring->setBrs(_params.enable_brs);

    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Starting to fetch measurements now.");
    _last_tof_measurement_timestamp     = std::chrono::steady_clock::now();
    _last_thermal_measurement_timestamp = std::chrono::steady_clock::now();

    // state transition
    _measurement_state = MeasurementState::extra_actions;
    break;
  }

    /* =============================================
      Loop part of the state machine
      Runs continuously to fetch data
    ============================================= */

  case MeasurementState::extra_actions: {
    // Execute all queued extra actions once per loop.
    std::queue<std::function<void()> > actions;
    {
      std::lock_guard<std::mutex> lock(_extra_actions_mutex);
      std::swap(actions, _extra_actions);
    }

    while (!actions.empty()) {
      auto& act = actions.front();
      if (act) {
        try {
          act();
        } catch (const std::exception& e) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception in MeasurementManager extra action: " + std::string(e.what()));
        } catch (...) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unknown exception in MeasurementManager extra action.");
        }
      }
      actions.pop();
    }

    // state transition
    _measurement_state = MeasurementState::request_tof_measurement;
    break;
  }

  case MeasurementState::request_tof_measurement: {
    if (_tof_enabled) {
      auto tof_devices = _device_groups.at(DeviceGroup::VL53L8CX).getDevicesOfType<device::VL53L8CX_Device>();
      if (!tof_devices.empty()) {
        _measurement_futures[MeasurementFutureKey::ToFRequest] = device::VL53L8CX_Device::requestTofMeasurementAsync(tof_devices, _params.timeout);
      }
    }
    _last_tof_measurement_timestamp = std::chrono::steady_clock::now();

    // state transition
    _measurement_state = MeasurementState::request_thermal_measurement;
    break;
  }

  case MeasurementState::request_thermal_measurement: {
    if (_thermal_enabled && !_thermal_measurement_flag) {
      bool measure_thermal = true;
      if (_is_thermal_throttled) {
        if ((std::chrono::steady_clock::now() - _last_thermal_measurement_timestamp) < _thermal_measurement_period)
          measure_thermal = false;
      }
      if (measure_thermal) {
        auto thermal_devices = _device_groups.at(DeviceGroup::HTPA32).getDevicesOfType<device::HTPA32_Device>();
        if (!thermal_devices.empty()) {
          _measurement_futures[MeasurementFutureKey::ThermalRequest] = device::HTPA32_Device::requestThermalMeasurementAsync(thermal_devices, _params.timeout);
        }
        _last_thermal_measurement_timestamp = std::chrono::steady_clock::now();
        _thermal_measurement_flag           = true;
      }
    }

    // state transition
    _measurement_state = MeasurementState::wait_for_data;
    break;
  }

  case MeasurementState::wait_for_data: {
    if (_is_tof_throttled || _first_measurement) {
      if (_tof_enabled)
        success &= waitForMeasurementFuture(MeasurementFutureKey::ToFRequest, _params.timeout);
    }

    // state transition
    if (success) {
      if (_first_measurement) {
        _first_measurement = false;
        _measurement_state = MeasurementState::extra_actions;
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
    // fetch and publish a tof measurement
    if (_tof_enabled) {

      auto tof_devices = _device_groups.at(DeviceGroup::VL53L8CX).getDevicesOfType<device::VL53L8CX_Device>();
      if (!tof_devices.empty()) {
        auto fut = device::VL53L8CX_Device::fetchTofMeasurementAsync(tof_devices, _params.timeout);
        if (fut.wait_for(_params.timeout) == std::future_status::ready) {
          success = fut.get();
        } else {
          success = false;
        }
      }

      if (success) {
        int error = notifyToFData();
        if (error != 0)
          logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Error occurred while parsing tof measurements from " + std::to_string(error) + " sensor(s)");
      }
    }

    // state transition
    if (success) {
      _measurement_state = MeasurementState::fetch_thermal_data;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occurred while fetching tof measurements.");
      _measurement_state = MeasurementState::error_handler_measurement;
    }
    break;
  }

  case MeasurementState::fetch_thermal_data: {
    // fetch and publish a thermal measurement
    if (_thermal_enabled && _thermal_measurement_flag) {

      auto thermal_devices = _device_groups.at(DeviceGroup::HTPA32).getDevicesOfType<device::HTPA32_Device>();
      if (!thermal_devices.empty()) {
        auto fut = device::HTPA32_Device::fetchThermalMeasurementAsync(thermal_devices, _params.timeout);
        success  = fut.wait_for(_params.timeout) == std::future_status::ready && fut.get();
      }

      if (success) {
        int error = notifyThermalData();
        if (error != 0)
          logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Error occurred while parsing thermal measurements from " + std::to_string(error) + " sensor(s)");
      }
      _thermal_measurement_flag = false;
    }

    // state transition
    if (success) {
      _measurement_state = MeasurementState::throttle_measurement;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occurred while fetching thermal measurements.");
      _measurement_state = MeasurementState::error_handler_measurement;
    }
    break;
  }

  case MeasurementState::throttle_measurement: {
    if (_tof_enabled && _is_tof_throttled) {
      std::this_thread::sleep_until(_last_tof_measurement_timestamp + _tof_measurement_period);
    } else {
      success &= waitForMeasurementFuture(MeasurementFutureKey::ToFRequest, _params.timeout);
    }

    // state transition
    if (success) {
      _measurement_state = MeasurementState::extra_actions;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occurred while taking tof measurements.");
      _measurement_state = MeasurementState::error_handler_measurement;
    }
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
      // Try to fix the error
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Trying to restart measurements.");

      if (success) {
        attempts = 0;
        for (device::IDevice* d : _sensor_ring->getDevices())
          if (auto bs = dynamic_cast<device::BaseSensor*>(d))
            bs->resetSensorState();
        do {
          attempts++;
          auto tof_devices = _device_groups.at(DeviceGroup::VL53L8CX).getDevicesOfType<device::VL53L8CX_Device>();
          if (!tof_devices.empty()) {
            _measurement_futures[MeasurementFutureKey::ToFRequest] = device::VL53L8CX_Device::requestTofMeasurementAsync(tof_devices, _params.timeout);
          }
          success = waitForMeasurementFuture(MeasurementFutureKey::ToFRequest, _params.timeout);
        } while (!success && _is_running && (attempts < 10));
      }
    }

    // state transition
    if (_params.repair_errors) {
      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting measurements succeeded after " + std::to_string(attempts) + " attempts.");
        _measurement_state = MeasurementState::extra_actions;
        notifyState(ManagerState::Running);
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to restart measurements. Resetting all sensors.");
        _measurement_state = MeasurementState::reset_sensors;
      }
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Will not attempt to restart measurements because parameter \"repair_errors\" is set to \"false\".");
      _measurement_state = MeasurementState::shutdown;
    }
    break;
  }

  case MeasurementState::error_handler_communication: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error handler for communication errors called.");
    notifyState(ManagerState::Error);

    unsigned int attempts = 0;

    if (_params.repair_errors) {
      // Try to fix the error

      bool communication_error = false;
      for (auto& bus : _sensor_ring->getInterfaces()) {
        auto interface = bus->getInterface();
        communication_error |= interface->hasError();
      }

      if (communication_error) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Communication error detected. Trying to restart affected interfaces.");

        do {
          attempts++;
          success = true;
          for (auto& bus : _sensor_ring->getInterfaces()) {
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

    // state transition
    if (_params.repair_errors) {
      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting communication succeeded after " + std::to_string(attempts) + " attempts.");
        _measurement_state = MeasurementState::extra_actions;
        notifyState(ManagerState::Running);
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to restart communication. Please check the interfaces.");
        _measurement_state = MeasurementState::shutdown;
      }
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Will not attempt to restart measurements because parameter \"repair_errors\" is set to \"false\".");
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

} // namespace manager

} // namespace eduart