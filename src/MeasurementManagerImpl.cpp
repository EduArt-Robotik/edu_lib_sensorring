#include "MeasurementManagerImpl.hpp"

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "sensorring/MeasurementClient.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/device/IDevice.hpp"
#include "sensorring/logger/Logger.hpp"

#include "SensorBoard.hpp"
#include "SensorBus.hpp"

namespace eduart {

namespace manager {

MeasurementManagerImpl::MeasurementManagerImpl(ManagerParams params)
    : _params(params)
    , _manager_state(ManagerState::Uninitialized)
    , _measurement_state(MeasurementState::init)
    , _sensor_ring(ring::SensorRing::create(_params.ring_params))
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
    , _light_mode(light::LightMode::Off)
    , _light_color{ 0, 0, 0 }
    , _light_brightness(0)
    , _light_update_flag(false)
    , _is_running(false)
    , _tof_device_group(device::DeviceGroup::createFromDevicesOfType<device::VL53L8CX_Device>(_sensor_ring->getDevices()))
    , _thermal_device_group(device::DeviceGroup::createFromDevicesOfType<device::HTPA32_Device>(_sensor_ring->getDevices()))
    , _light_device_group(device::DeviceGroup::createFromDevicesOfType<device::WS2812b_Device>(_sensor_ring->getDevices())) {

  // check if there are active tof or thermal sensors (device-group based)
  _tof_device_group.invokeForEachDevice([this](device::BaseDevice* device) {
    if (static_cast<device::BaseSensor*>(device)->getEnable())
      _tof_enabled = true;
  });
  _thermal_device_group.invokeForEachDevice([this](device::BaseDevice* device) {
    if (static_cast<device::BaseSensor*>(device)->getEnable())
      _thermal_enabled = true;
  });

  // prepare state machine
  _manager_state = ManagerState::Initialized;
}

MeasurementManagerImpl::~MeasurementManagerImpl() noexcept {
  stopMeasuring();
}

void MeasurementManagerImpl::enableTofMeasurement(bool state) noexcept {
  _tof_enabled = state;
}

void MeasurementManagerImpl::enableThermalMeasurement(bool state) noexcept {
  _thermal_enabled = state;
}

ManagerParams MeasurementManagerImpl::getParams() const noexcept {
  return _params;
}

std::string MeasurementManagerImpl::printTopology() const noexcept {
  std::stringstream ss;
  for (const auto& bus : _sensor_ring->getInterfaces()) {
    ss << std::endl << std::endl;
    ss << "=================================================" << std::endl;
    ss << "Topology of the sensors on " << bus->getInterface()->getInterfaceName() << ":" << std::endl;
    ss << std::endl;

    auto enum_info_vec = bus->getEnumerationInfo();
    for (const auto& enum_info : enum_info_vec) {
      auto board_infos = device::SensorBoardManager::getSensorBoardInfo(enum_info.type);

      ss << "sensor " << enum_info.idx << std::endl;
      ss << "    Type:           " << board_infos.name << std::endl;
      ss << "    State:          " << toString(enum_info.state) << std::endl;
      ss << "    FW revision:    " << enum_info.version << " (" << enum_info.hash << ")" << std::endl;
      ss << "    ToF sensor:     " << board_infos.tof.name << std::endl;
      ss << "    Thermal sensor: " << board_infos.thermal.name << std::endl;
      ss << "    RGB LEDs:       " << board_infos.leds.name << std::endl;
      ss << "    Nr of LEDs:     " << board_infos.leds.count << std::endl;
      ss << std::endl;
    }

    ss << "=================================================" << std::endl;
  }
  return ss.str();
}

bool MeasurementManagerImpl::stopThermalCalibration() noexcept {
  auto success = true;
  _thermal_device_group.invokeForEachDevice([&success](device::BaseDevice* device) {
    auto res = device->invoke<device::StopCalibration>({});
    if (!res || !res->success) {
      success = false;
    }
  });
  return success;
}

bool MeasurementManagerImpl::startThermalCalibration(std::size_t window) noexcept {
  auto success = true;
  _thermal_device_group.invokeForEachDevice([&success, window](device::BaseDevice* device) {
    auto res = device->invoke<device::StartCalibration>({ window });
    if (!res || !res->success) {
      success = false;
    }
  });
  return success;
}

void MeasurementManagerImpl::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept {
  _light_mode     = mode;
  _light_color[0] = red;
  _light_color[1] = green;
  _light_color[2] = blue;

  _light_update_flag = true;
}

/* =======================================================================================
        Handle clients
==========================================================================================
*/

void MeasurementManagerImpl::registerClient(MeasurementClient* client) {
  if (client) {
    LockGuard lock(_client_mutex);
    auto result = _clients.insert(client);

    // Check if the client was registered
    if (result.second) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Registered new measurement client");
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Measurement client is already registered");
    }
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Measurement client to be registered is not valid");
  }
}

void MeasurementManagerImpl::unregisterClient(MeasurementClient* client) {
  if (client) {
    LockGuard lock(_client_mutex);
    auto result = _clients.erase(client);

    // Check if the client was removed
    if (result > 0) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Removed measurement client");
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Measurement client to be removed is not registered");
    }
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Measurement client to be removed is not valid");
  }
}

bool MeasurementManagerImpl::waitForTofMeasurementFutures(std::chrono::steady_clock::duration timeout) noexcept {
  return device::DeviceGroup::waitForAll(_tof_measurement_futures, timeout, [](const device::RequestTofMeasurement::Response& r) {
    return r.ready;
  });
}

bool MeasurementManagerImpl::waitForTofFetchFutures(std::chrono::steady_clock::duration timeout) noexcept {
  return device::DeviceGroup::waitForAll(_tof_fetch_futures, timeout, [](const device::FetchTofMeasurement::Response& r) {
    return r.complete;
  });
}

bool MeasurementManagerImpl::waitForThermalMeasurementFutures(std::chrono::steady_clock::duration timeout) noexcept {
  return device::DeviceGroup::waitForAll(_thermal_measurement_futures, timeout, [](const device::RequestThermalMeasurement::Response& r) {
    return r.ready;
  });
}

bool MeasurementManagerImpl::waitForThermalFetchFutures(std::chrono::steady_clock::duration timeout) noexcept {
  return device::DeviceGroup::waitForAll(_thermal_fetch_futures, timeout, [](const device::FetchThermalMeasurement::Response& r) {
    return r.complete;
  });
}

int MeasurementManagerImpl::notifyToFData() {
  int error_frames = 0;
  std::vector<measurement::TofMeasurement> raw_measurement_vec, transformed_measurement_vec;

  _tof_device_group.invokeForEachDevice([&error_frames, &raw_measurement_vec, &transformed_measurement_vec](device::BaseDevice* device) {
    if (!static_cast<device::BaseSensor*>(device)->getEnable())
      return;
    auto raw_opt = device->invoke<device::GetLatestRawMeasurement>({});
    if (raw_opt && raw_opt->state == device::SensorState::SensorOK) {
      if (!raw_opt->measurement.point_cloud.data.empty())
        raw_measurement_vec.emplace_back(raw_opt->measurement);
    } else {
      error_frames++;
    }
    auto transformed_opt = device->invoke<device::GetLatestTransformedMeasurement>({});
    if (transformed_opt && transformed_opt->state == device::SensorState::SensorOK) {
      if (!transformed_opt->measurement.point_cloud.data.empty())
        transformed_measurement_vec.emplace_back(transformed_opt->measurement);
    }
  });

  if (!raw_measurement_vec.empty()) {
    LockGuard lock(_client_mutex);
    for (auto client : _clients) {
      if (client)
        client->onRawTofMeasurement(raw_measurement_vec);
    }
  }

  if (!transformed_measurement_vec.empty()) {
    LockGuard lock(_client_mutex);
    for (auto client : _clients) {
      if (client)
        client->onTransformedTofMeasurement(transformed_measurement_vec);
    }
  }

  return error_frames;
}

int MeasurementManagerImpl::notifyThermalData() {
  int error_frames = 0;
  std::vector<measurement::ThermalMeasurement> measurement_vec;

  _thermal_device_group.invokeForEachDevice([&error_frames, &measurement_vec](device::BaseDevice* device) {
    if (!static_cast<device::BaseSensor*>(device)->getEnable())
      return;
    auto resp_opt = device->invoke<device::GetLatestMeasurement>({});
    if (resp_opt && resp_opt->state == device::SensorState::SensorOK) {
      measurement_vec.emplace_back(resp_opt->measurement);
    } else {
      error_frames++;
    }
  });

  if (!measurement_vec.empty()) {
    LockGuard lock(_client_mutex);
    for (auto client : _clients) {
      if (client)
        client->onThermalMeasurement(measurement_vec);
    }
  }

  return error_frames;
}

void MeasurementManagerImpl::notifyState(const ManagerState state) {
  LockGuard lock(_client_mutex);
  if (_manager_state != state) {
    _manager_state = state;
    for (auto client : _clients) {
      if (client)
        client->onStateChange(state);
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
            Runs once at start and may be triggered again on error conditions
    ============================================= */

  case MeasurementState::init: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManager state machine");

    // state transition
    _measurement_state = MeasurementState::reset_sensors;
    break;
  }

  case MeasurementState::reset_sensors: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
    _sensor_ring->resetDevices();
    std::this_thread::sleep_for(std::chrono::seconds(2)); // sleep 2 seconds -> boards need time to init their vl53l8 sensors

    // state transition
    _measurement_state = MeasurementState::sync_lights;
    break;
  }

  case MeasurementState::sync_lights: {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Syncing all lights and set to mode pulsation");

    device::IDevice::static_invoke<device::WS2812b_Device, device::SyncLight>({});
    device::IDevice::static_invoke<device::WS2812b_Device, device::SetLight>({ light::LightMode::Pulsation, 0, 0, 0 });

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
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, printTopology());
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

      _thermal_device_group.invokeForEachDevice([&success](device::BaseDevice* device) {
        auto res = device->invoke<device::GetEPROM>({});
        if (!res || !res->success) {
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
    _measurement_state = MeasurementState::set_lights;
    break;
  }

    /* =============================================
            Loop part of the state machine
            Runs continuously to fetch data
    ============================================= */

  case MeasurementState::set_lights: {
    if (_light_update_flag) {
      device::IDevice::static_invoke<device::WS2812b_Device, device::SetLight>({ _light_mode, _light_color[0], _light_color[1], _light_color[2] });
      _light_update_flag = false;
    }

    // state transition
    _measurement_state = MeasurementState::request_tof_measurement;
    break;
  }

  case MeasurementState::request_tof_measurement: {
    if (_tof_enabled) {
      _tof_measurement_futures.clear();
      const auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_params.ring_params.timeout);
      _tof_device_group.invokeForEachDevice([this, timeout_ms](device::BaseDevice* device) {
        if (!static_cast<device::BaseSensor*>(device)->getEnable())
          return;
        auto opt = device->invoke_async<device::RequestTofMeasurement>({ timeout_ms });
        if (opt)
          _tof_measurement_futures.push_back(std::move(*opt));
      });
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
        _thermal_measurement_futures.clear();
        const auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_params.ring_params.timeout);
        _thermal_device_group.invokeForEachDevice([this, timeout_ms](device::BaseDevice* device) {
          if (!static_cast<device::BaseSensor*>(device)->getEnable())
            return;
          auto opt = device->invoke_async<device::RequestThermalMeasurement>({ timeout_ms });
          if (opt)
            _thermal_measurement_futures.push_back(std::move(*opt));
        });
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
        success &= waitForTofMeasurementFutures(_params.ring_params.timeout);
    }

    // state transition
    if (success) {
      if (_first_measurement) {
        _first_measurement = false;
        _measurement_state = MeasurementState::set_lights;
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
      _tof_fetch_futures.clear();
      const auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_params.ring_params.timeout);
      _tof_device_group.invokeForEachDevice([this, timeout_ms](device::BaseDevice* device) {
        if (!static_cast<device::BaseSensor*>(device)->getEnable())
          return;
        auto opt = device->invoke_async<device::FetchTofMeasurement>({ timeout_ms });
        if (opt)
          _tof_fetch_futures.push_back(std::move(*opt));
      });
      success = waitForTofFetchFutures(_params.ring_params.timeout);
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
      _thermal_fetch_futures.clear();
      const auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_params.ring_params.timeout);
      _thermal_device_group.invokeForEachDevice([this, timeout_ms](device::BaseDevice* device) {
        if (!static_cast<device::BaseSensor*>(device)->getEnable())
          return;
        auto opt = device->invoke_async<device::FetchThermalMeasurement>({ timeout_ms });
        if (opt)
          _thermal_fetch_futures.push_back(std::move(*opt));
      });
      success = waitForThermalFetchFutures(_params.ring_params.timeout);
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
    if (_tof_enabled && _is_tof_throttled)
      std::this_thread::sleep_until(_last_tof_measurement_timestamp + _tof_measurement_period);

    // request futures already completed in wait_for_data (measurement ready); no second wait

    // state transition
    if (success) {
      _measurement_state = MeasurementState::set_lights;
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
        for (device::BaseDevice* d : _sensor_ring->getDevices())
          static_cast<device::BaseSensor*>(d)->resetSensorState();
        do {
          attempts++;
          _tof_measurement_futures.clear();
          const auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_params.ring_params.timeout);
          _tof_device_group.invokeForEachDevice([this, timeout_ms](device::BaseDevice* device) {
            if (!static_cast<device::BaseSensor*>(device)->getEnable())
              return;
            auto opt = device->invoke_async<device::RequestTofMeasurement>({ timeout_ms });
            if (opt)
              _tof_measurement_futures.push_back(std::move(*opt));
          });
          success = waitForTofMeasurementFutures(_params.ring_params.timeout);
        } while (!success && _is_running && (attempts < 10));
      }
    }

    // state transition
    if (_params.repair_errors) {
      if (success) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Restarting measurements succeeded after " + std::to_string(attempts) + " attempts.");
        _measurement_state = MeasurementState::set_lights;
        _light_update_flag = true;
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
        _measurement_state = MeasurementState::set_lights;
        _light_update_flag = true;
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