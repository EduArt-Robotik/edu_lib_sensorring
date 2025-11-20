#include "MeasurementManagerImpl.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "boardmanager/SensorBoardManager.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/MeasurementClient.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/logger/Logger.hpp"

#include "SensorBoard.hpp"
#include "SensorBus.hpp"
#include "SensorRing.hpp"

namespace eduart {

namespace manager {

MeasurementManagerImpl::MeasurementManagerImpl(ManagerParams params)
    : _params(params)
    , _manager_state(ManagerState::Uninitialized)
    , _measurement_state(MeasurementState::init)
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
    , _is_running(false) {

  // Assemble the sensor ring
  std::vector<std::unique_ptr<bus::SensorBus> > bus_vec;
  for (const auto& bus_params : params.ring_params.bus_param_vec) {
    auto interface = com::ComManager::getInstance()->createInterface(bus_params.interface_name, bus_params.type);

    unsigned int idx = 0;
    std::vector<std::unique_ptr<sensor::SensorBoard> > board_vec;
    for (const auto& board_params : bus_params.board_param_vec) {
      auto tof     = std::make_unique<sensor::TofSensor>(board_params.tof_params, interface, idx);
      auto thermal = std::make_unique<sensor::ThermalSensor>(board_params.thermal_params, interface, idx);
      auto light   = std::make_unique<sensor::LedLight>(board_params.led_params);

      board_vec.push_back(std::make_unique<sensor::SensorBoard>(board_params, interface, idx, std::move(tof), std::move(thermal), std::move(light)));
      idx++;
    }

    bus_vec.push_back(std::make_unique<bus::SensorBus>(interface, std::move(board_vec)));
  }
  _sensor_ring = std::make_unique<ring::SensorRing>(params.ring_params, std::move(bus_vec));

  // check if there are active tof or thermal sensors
  for (const auto& sensor_bus : _sensor_ring->getInterfaces()) {
    for (unsigned int j = 0; j < sensor_bus->getSensorCount(); j++) {
      _tof_enabled |= sensor_bus->isTofEnabled(j);
      _thermal_enabled |= sensor_bus->isThermalEnabled(j);
    }
  }

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
      auto board_infos = sensor::SensorBoardManager::getSensorBoardInfo(enum_info.type);

      ss << "sensor " << enum_info.idx << std::endl;
      ss << "    Type:           " << board_infos.name << std::endl;
      ss << "    State:          " << toString(enum_info.state) << std::endl;
      ss << "    FW revision:    " << enum_info.version << " (" << enum_info.hash << ")" << std::endl;
      ss << "    ToF sensor:     " << board_infos.tof.name << std::endl;
      ss << "    Thermal sensor: " << board_infos.thermal.name << std::endl;
      ss << "    Nr of LEDs:     " << board_infos.leds.name << std::endl;
      ss << std::endl;
    }

    ss << "=================================================" << std::endl;
  }
  return ss.str();
}

bool MeasurementManagerImpl::stopThermalCalibration() noexcept {
  return _sensor_ring->stopThermalCalibration();
}

bool MeasurementManagerImpl::startThermalCalibration(std::size_t window) noexcept {
  return _sensor_ring->startThermalCalibration(window);
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

void MeasurementManagerImpl::registerClient(MeasurementClient* client) noexcept {
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

void MeasurementManagerImpl::unregisterClient(MeasurementClient* client) noexcept {
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

int MeasurementManagerImpl::notifyToFData() {
  int error_frames = 0;
  std::vector<measurement::TofMeasurement> raw_measurement_vec, transformed_measurement_vec;

  for (const auto& sensor_bus : _sensor_ring->getInterfaces()) {
    for (const auto& sensor_board : sensor_bus->getSensorBoards()) {
      if (sensor_board->getTof()->getEnable()) {
        auto [raw_measurement, raw_error] = sensor_board->getTof()->getLatestRawMeasurement();
        if (raw_error == sensor::SensorState::SensorOK) {
          if (!raw_measurement.point_cloud.empty())
            raw_measurement_vec.push_back(raw_measurement);
        } else {
          error_frames++;
        }

        auto [transformed_measurement, transformed_error] = sensor_board->getTof()->getLatestTransformedMeasurement();
        if (transformed_error == sensor::SensorState::SensorOK) {
          if (!transformed_measurement.point_cloud.empty())
            transformed_measurement_vec.push_back(transformed_measurement);
        }
      }
    }
  }

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

  for (const auto& sensor_bus : _sensor_ring->getInterfaces()) {
    for (const auto& sensor_board : sensor_bus->getSensorBoards()) {
      if (sensor_board->getThermal()->getEnable()) {
        auto [measurement, error] = sensor_board->getThermal()->getLatestMeasurement();

        if (error == sensor::SensorState::SensorOK) {
          measurement_vec.push_back(measurement);
        } else {
          error_frames++;
        }
      }
    }
  }

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
  if (_manager_state != state) {
    LockGuard lock(_client_mutex);
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

bool MeasurementManagerImpl::measureSome() noexcept(false) {
  bool error = false;

  if (!_is_running) {
    if (_tof_enabled || _thermal_enabled) {
      notifyState(ManagerState::Running);
      StateMachine();
      error = true;
    }
  }

  return error;
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
void MeasurementManagerImpl::StateMachineWorker() {
  while (_is_running) {
    // no wait command here, the individual states of the state machine
    // provide natural throttling
    try {
      StateMachine();
    } catch (std::runtime_error& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Caught communication error: " + std::string(e.what()));
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
    _sensor_ring->syncLight();
    _sensor_ring->setLight(light::LightMode::Off, 0, 0, 0);

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
      success = _sensor_ring->getEEPROM();
    }

    // state transition
    if (success) {
      _measurement_state = MeasurementState::pre_loop_init;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to read EEPROM values from at least one sensor");
      _measurement_state = MeasurementState::error_handler_measurement;
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
      _sensor_ring->setLight(_light_mode, _light_color[0], _light_color[1], _light_color[2]);
      _light_update_flag = false;
    }

    // state transition
    _measurement_state = MeasurementState::request_tof_measurement;
    break;
  }

  case MeasurementState::request_tof_measurement: {
    if (_tof_enabled)
      _sensor_ring->requestTofMeasurement();
    _last_tof_measurement_timestamp = std::chrono::steady_clock::now();

    // state transition
    _measurement_state = MeasurementState::request_thermal_measurement;
    break;
  }

  case MeasurementState::request_thermal_measurement: {
    if (_thermal_enabled) {
      if (!_thermal_measurement_flag) {
        bool measure_thermal = true;
        if (_is_thermal_throttled) {
          if ((std::chrono::steady_clock::now() - _last_thermal_measurement_timestamp) < _thermal_measurement_period) {
            measure_thermal = false;
          }
        }

        if (measure_thermal) {
          _sensor_ring->requestThermalMeasurement();
          _last_thermal_measurement_timestamp = std::chrono::steady_clock::now();
          _thermal_measurement_flag           = true;
        }
      }
    }

    // state transition
    _measurement_state = MeasurementState::wait_for_data;
    break;
  }

  case MeasurementState::wait_for_data: {
    // wait for the completion of measurements if a frequency was specified
    // or this is the first measurement
    if (_is_tof_throttled || _first_measurement) {
      if (_tof_enabled)
        success &= _sensor_ring->waitForAllTofMeasurementsReady();
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
      _sensor_ring->fetchTofMeasurement();
      success = _sensor_ring->waitForAllTofDataTransmissionsComplete();
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
      _sensor_ring->fetchThermalMeasurement();
      success = _sensor_ring->waitForAllThermalDataTransmissionsComplete();
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
      // throttled mode: wait until next measurement period
      std::this_thread::sleep_until(_last_tof_measurement_timestamp + _tof_measurement_period);
    }

    success &= _sensor_ring->waitForAllTofMeasurementsReady();

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
        _sensor_ring->resetSensorState();
        do {
          attempts++;
          _sensor_ring->requestTofMeasurement();
          success = _sensor_ring->waitForAllTofMeasurementsReady();
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