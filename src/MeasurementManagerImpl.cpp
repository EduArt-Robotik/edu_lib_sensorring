#include "MeasurementManagerImpl.hpp"

#include <memory>
#include <sstream>
#include <algorithm>

#include "Parameters.hpp"
#include "SensorBoard.hpp"
#include "SensorBus.hpp"
#include "SensorRing.hpp"
#include "logger/Logger.hpp"

namespace eduart {

namespace manager {

MeasurementManagerImpl::MeasurementManagerImpl(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring) : _params(params) {
  init(std::move(sensor_ring));
}

MeasurementManagerImpl::~MeasurementManagerImpl() { stopMeasuring(); }

void MeasurementManagerImpl::init(std::unique_ptr<ring::SensorRing> sensor_ring) {
  _sensor_ring = std::move(sensor_ring);

  _is_tof_throttled = _params.frequency_tof_hz > 0.0;
  _is_thermal_throttled = _params.frequency_thermal_hz > 0.0;

  // check if there are active tof or thermal sensors
  _tof_enabled = false;
  _thermal_enabled = false;
  for (const auto &sensor_bus : _sensor_ring->getInterfaces()) {
    for (size_t j = 0; j < sensor_bus->getSensorCount(); j++) {
      _tof_enabled |= sensor_bus->isTofEnabled(j);
      _thermal_enabled |= sensor_bus->isThermalEnabled(j);
    }
  }

  _is_running = false;
  _first_measurement = true;
  _thermal_measurement_flag = false;

  _light_mode = light::LightMode::Off;
  _light_brightness = 0;
  _light_color[0] = 0;
  _light_color[1] = 0;
  _light_color[2] = 0;
  _light_update_flag = false;

  _last_tof_measurement_timestamp_s = std::chrono::steady_clock::now();
  _last_thermal_measurement_timestamp_s = std::chrono::steady_clock::now();

  if (_params.print_topology) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Info, printTopology());
  }

  // prepare state machine
  notifyState(WorkerState::Initialized);
  _measurement_state = MeasurementState::init;
}

void MeasurementManagerImpl::enableTofMeasurement(bool state) { _tof_enabled = state; }

void MeasurementManagerImpl::enableThermalMeasurement(bool state) { _thermal_enabled = state; }

ManagerParams MeasurementManagerImpl::getParams() const { return _params; }

std::string MeasurementManagerImpl::printTopology() const {
  std::stringstream ss;
  for (const auto &sensor_bus : _sensor_ring->getInterfaces()) {
    ss << "";
    ss << "=================================================";
    ss << "Topology of the sensors on " << sensor_bus->getInterface()->getInterfaceName() << ":";
    ss << "";
    for (const auto &sensor_board : sensor_bus->getSensorBoards()) {
      std::string board_type = "";
      std::string tof_sensor = "";
      std::string thermal_sensor = "";
      std::string led_count = "";
      switch (sensor_board->getType()) {
        case sensor::SensorBoardType::headlight:
          board_type = "Headlight";
          tof_sensor = "VL53L8CX";
          thermal_sensor = "HTPA32";
          led_count = "11";
          break;
        case sensor::SensorBoardType::taillight:
          board_type = "Taillight";
          tof_sensor = "VL53L8CX";
          thermal_sensor = "none";
          led_count = "8";
          break;
        case sensor::SensorBoardType::sidepanel:
          board_type = "Sidepanel";
          tof_sensor = "VL53L8CX";
          thermal_sensor = "none";
          led_count = "2";
          break;
        case sensor::SensorBoardType::minipanel:
          board_type = "Minipanel";
          tof_sensor = "VL53L8CX";
          thermal_sensor = "none";
          led_count = "none";
          break;
        case sensor::SensorBoardType::unknown:
        default:
          board_type = "unknown";
          tof_sensor = "n.a.";
          thermal_sensor = "n.a.";
          led_count = "n.a.";
          break;
      }

      ss << "sensor " << sensor_board->getTof()->getIdx() << " is a " << board_type;
      ss << "		ToF sensor: " << tof_sensor;
      ss << "		Thermal sensor: " << thermal_sensor;
      ss << "		Nr of LEDs: " << led_count;
    }
    ss << "=================================================";
  }
  ss << "";
  return ss.str();
}

bool MeasurementManagerImpl::stopThermalCalibration() { return _sensor_ring->stopThermalCalibration(); }

bool MeasurementManagerImpl::startThermalCalibration(std::size_t window) { return _sensor_ring->startThermalCalibration(window); }

void MeasurementManagerImpl::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  _light_mode = mode;
  _light_color[0] = red;
  _light_color[1] = green;
  _light_color[2] = blue;

  _light_update_flag = true;
}

/* =======================================================================================
        Handle observers
==========================================================================================
*/

void MeasurementManagerImpl::registerClient(MeasurementClient *observer) {
  if (observer) {
    // check if the observer is already registered
    if (std::find(_observer_vec.begin(), _observer_vec.end(), observer) == _observer_vec.end()) {
      _observer_vec.push_back(observer);
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Registered new observer");
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Observer already registered");
    }
  }
}

void MeasurementManagerImpl::unregisterClient(MeasurementClient *observer) {
  if (observer) {
    // check if the observer is already registered
    const auto &it = std::find(_observer_vec.begin(), _observer_vec.end(), observer);
    if (it != _observer_vec.end()) {
      _observer_vec.erase(it);
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Registered new observer");
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Observer already registered");
    }
  }
}

int MeasurementManagerImpl::notifyToFData() {
  int error_frames = 0;
  std::vector<measurement::TofMeasurement> raw_measurement_vec, tansformed_measurement_vec;

  for (const auto &sensor_bus : _sensor_ring->getInterfaces()) {
    for (const auto &sensor_board : sensor_bus->getSensorBoards()) {
      if (sensor_board->getTof()->isEnabled()) {
        auto [raw_measurement, raw_error] = sensor_board->getTof()->getLatestRawMeasurement();
        if (raw_error == sensor::SensorState::SensorOK) {
          if (!raw_measurement.point_cloud.empty()) raw_measurement_vec.push_back(raw_measurement);
        } else {
          error_frames++;
        }

        auto [tansformed_measurement, transformed_error] = sensor_board->getTof()->getLatestTransformedMeasurement();
        if (transformed_error == sensor::SensorState::SensorOK) {
          if (!tansformed_measurement.point_cloud.empty()) tansformed_measurement_vec.push_back(tansformed_measurement);
        }
      }
    }
  }

  if (!raw_measurement_vec.empty()) {
    for (auto observer : _observer_vec) {
      if (observer) observer->onRawTofMeasurement(raw_measurement_vec);
    }
  }

  if (!tansformed_measurement_vec.empty()) {
    for (auto observer : _observer_vec) {
      if (observer) observer->onTransformedTofMeasurement(tansformed_measurement_vec);
    }
  }

  return error_frames;
}

int MeasurementManagerImpl::notifyThermalData() {
  int error_frames = 0;
  std::vector<measurement::ThermalMeasurement> measurement_vec;

  for (const auto &sensor_bus : _sensor_ring->getInterfaces()) {
    for (const auto &sensor_board : sensor_bus->getSensorBoards()) {
      if (sensor_board->getThermal()->isEnabled()) {
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
    for (auto observer : _observer_vec) {
      if (observer) observer->onThermalMeasurement(measurement_vec);
    }
  }

  return error_frames;
}

void MeasurementManagerImpl::notifyState(const WorkerState state) {
  _manager_state = state;

  for (auto observer : _observer_vec) {
    if (observer) observer->onStateChange(state);
  }
}

WorkerState MeasurementManagerImpl::getWorkerState() const { return _manager_state; }

/* =======================================================================================
        Start and stop measurements
==========================================================================================
*/

bool MeasurementManagerImpl::measureSome() {
  bool error = false;

  if (!_is_running) {
    if (_tof_enabled || _thermal_enabled) {
      notifyState(WorkerState::Running);
      StateMachine();
      error = true;
    }
  }

  return error;
}

bool MeasurementManagerImpl::startMeasuring() {
  bool error = false;

  if (!_is_running) {
    if (_tof_enabled || _thermal_enabled) {
      // begin state machine operation
      _is_running = true;
      _worker_thread = std::thread(&MeasurementManagerImpl::StateMachineWorker, this);
      notifyState(WorkerState::Running);
      error = true;
    }
  }

  return error;
}

bool MeasurementManagerImpl::stopMeasuring() {
  bool error = false;

  if (_is_running) {
    _is_running = false;
    while (!_worker_thread.joinable()) {
    }

    _worker_thread.join();
    notifyState(WorkerState::Shutdown);
    error = true;
  }

  return error;
}

/* =======================================================================================
        State machine function
==========================================================================================
*/
void MeasurementManagerImpl::StateMachineWorker() {
  while (_is_running) {
    // no wait command here, the individual states of the state machine
    // provide natural throttling
    StateMachine();
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
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Initializing MeasurementManagerImpl state machine");

      // state transition
      _measurement_state = MeasurementState::reset_sensors;
      break;
    }

    case MeasurementState::reset_sensors: {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Resetting all connected sensors");
      _sensor_ring->resetDevices();
      std::this_thread::sleep_for(std::chrono::seconds(2));  // sleep 2 seconds -> boards need time
                                                             // to init their vl53l8 sensors!

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
        if (sensor_bus->getSensorCount() == sensor_bus->getEnumerationCount()) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Counted " + std::to_string(sensor_bus->getEnumerationCount()) + " of " +
                                                                             std::to_string(sensor_bus->getSensorCount()) + " sensors on interface " +
                                                                             sensor_bus->getInterface()->getInterfaceName());
        } else {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Counted " + std::to_string(sensor_bus->getEnumerationCount()) + " of " +
                                                                             std::to_string(sensor_bus->getSensorCount()) + " sensors on interface " +
                                                                             sensor_bus->getInterface()->getInterfaceName());
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, std::to_string(sensor_bus->getSensorCount()) +
                                                                              " sensors are specified in the parameters. Check topology and restart.");
        }
      }

      // state transition
      if (success) {
        _measurement_state = MeasurementState::get_eeprom;
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to enumerate sensors");
        _measurement_state = MeasurementState::error_handler;
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
        _measurement_state = MeasurementState::error_handler;
      }
      break;
    }

    case MeasurementState::pre_loop_init: {
      // enable bit rate switching
      _sensor_ring->setBRS(_params.enable_brs);

      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Starting to fetch measurements now ...");
      _last_tof_measurement_timestamp_s = std::chrono::steady_clock::now();
      _last_thermal_measurement_timestamp_s = std::chrono::steady_clock::now();

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
      if (_tof_enabled) _sensor_ring->requestTofMeasurement();
      _last_tof_measurement_timestamp_s = std::chrono::steady_clock::now();

      // state transition
      _measurement_state = MeasurementState::request_thermal_measurement;
      break;
    }

    case MeasurementState::request_thermal_measurement: {
      if (_thermal_enabled) {
        if (!_thermal_measurement_flag) {
          bool measure_thermal = true;
          if (_is_thermal_throttled) {
            if (std::chrono::duration<double>(1.0F / _params.frequency_thermal_hz) >
                (std::chrono::steady_clock::now() - _last_thermal_measurement_timestamp_s)) {
              measure_thermal = false;
            }
          }

          if (measure_thermal) {
            _sensor_ring->requestThermalMeasurement();
            _last_thermal_measurement_timestamp_s = std::chrono::steady_clock::now();
            _thermal_measurement_flag = true;
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
        if (_tof_enabled) success &= _sensor_ring->waitForAllTofMeasurementsReady();
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
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error,
                                           "Timeout occured while waiting for completion of "
                                           "measurements.");
        _measurement_state = MeasurementState::error_handler;
      }
      break;
    }

    case MeasurementState::fetch_tof_data: {
      // fetch and publish a tof measurement
      if (_tof_enabled) {
        _sensor_ring->fetchTofData();
        success = _sensor_ring->waitForAllTofDataTransmissionsComplete();
        if (success) {
          int error = notifyToFData();
          if (error != 0)
            logger::Logger::getInstance()->log(logger::LogVerbosity::Warning,
                                               "Error occured while parsing tof measurements from " + std::to_string(error) + " sensor(s)");
        }
      }

      // state transition
      if (success) {
        _measurement_state = MeasurementState::fetch_thermal_data;
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occured while fetching tof measurements.");
        _measurement_state = MeasurementState::error_handler;
      }
      break;
    }

    case MeasurementState::fetch_thermal_data: {
      // fetch and publish a thermal measurement
      if (_thermal_enabled and _thermal_measurement_flag) {
        _sensor_ring->fetchThermalData();
        success = _sensor_ring->waitForAllThermalDataTransmissionsComplete();
        if (success) {
          int error = notifyThermalData();
          if (error != 0)
            logger::Logger::getInstance()->log(logger::LogVerbosity::Warning,
                                               "Error occured while parsing thermal measurements from " + std::to_string(error) + " sensor(s)");
        }
        _thermal_measurement_flag = false;
      }

      // state transition
      if (success) {
        _measurement_state = MeasurementState::throttle_measurement;
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occured while fetching thermal measurements.");
        _measurement_state = MeasurementState::error_handler;
      }
      break;
    }

    case MeasurementState::throttle_measurement: {
      if (_is_tof_throttled) {
        // throttled mode: wait until next measurement period
        std::this_thread::sleep_until(_last_tof_measurement_timestamp_s + std::chrono::duration<double>(1.0F / _params.frequency_tof_hz));
      } else {
        // free running mode: wait until current measurements are finished
        if (_tof_enabled) success &= _sensor_ring->waitForAllTofMeasurementsReady();
      }

      // state transition
      if (success) {
        _measurement_state = MeasurementState::set_lights;
      } else {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Timeout occured while taking tof measurements.");
        _measurement_state = MeasurementState::error_handler;
      }
      break;
    }

      /* =============================================
              Error handler
      ============================================= */

    case MeasurementState::error_handler: {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error handler called.");
      notifyState(WorkerState::Error);
      _measurement_state = MeasurementState::shutdown;
      break;
    }

      /* =============================================
              Shutdown
      ============================================= */

    case MeasurementState::shutdown: {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Shutting down state machine.");
      notifyState(WorkerState::Shutdown);
      _is_running = false;
      break;
    }
  };
}




}  // namespace manager

}  // namespace eduart