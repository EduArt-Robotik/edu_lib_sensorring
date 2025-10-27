#include "SensorBus.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>

#include "interface/ComInterface.hpp"
#include "interface/can/canprotocol.hpp"
#include "logger/Logger.hpp"
#include "sensors/LedLight.hpp"
#include "sensors/ThermalSensor.hpp"
#include "sensors/TofSensor.hpp"

#include "SensorBoard.hpp"

namespace eduart {

namespace bus {

SensorBus::SensorBus(com::ComInterface* interface, std::vector<std::unique_ptr<sensor::SensorBoard> > board_vec)
    : _interface(interface)
    , _sensor_vec(std::move(board_vec))
    , _enumeration_flag(false)
    , _enumeration_count(0)
    , _active_tof_sensors(0)
    , _active_thermal_sensors(0)
    , _tof_measurement_count(0)
    , _thermal_measurement_count(0) {
  if (!_interface) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open com interface");
  }

  addEndpoint(com::ComEndpoint("broadcast"));
  addEndpoint(com::ComEndpoint("tof_status"));
  addEndpoint(com::ComEndpoint("thermal_status"));
  _interface->registerObserver(this);
}

SensorBus::~SensorBus() {
  _interface->unregisterObserver(this);
}

com::ComInterface* SensorBus::getInterface() const {
  return _interface;
}

std::vector<const sensor::SensorBoard*> SensorBus::getSensorBoards() const {

  std::vector<const sensor::SensorBoard*> ref_vec;
  for (const auto& sensor : _sensor_vec) {
    ref_vec.push_back(sensor.get());
  }

  return ref_vec;
}

bool SensorBus::isTofEnabled(int idx) const {
  if (idx >= 0 && idx < (int)_sensor_vec.size()) {
    return _sensor_vec[idx]->getTof()->isEnabled();
  }
  return false;
}

bool SensorBus::isThermalEnabled(int idx) const {
  if (idx >= 0 && idx < (int)_sensor_vec.size()) {
    return _sensor_vec[idx]->getThermal()->isEnabled();
  }
  return false;
}

size_t SensorBus::getSensorCount() const {
  return _sensor_vec.size();
}

size_t SensorBus::getEnumerationCount() const {
  return _enumeration_count;
}

void SensorBus::setBrs(bool brs_enable) {
  sensor::SensorBoard::cmdSetBrs(_interface, brs_enable);
}

void SensorBus::syncLight() {
  sensor::LedLight::cmdSyncLight(_interface);
}

void SensorBus::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  sensor::LedLight::cmdSetLight(_interface, mode, red, green, blue);
}

void SensorBus::resetDevices() {
  sensor::SensorBoard::cmdReset(_interface);
}

int SensorBus::enumerateDevices() {
  _enumeration_flag  = true;
  _enumeration_count = 0;

  sensor::SensorBoard::cmdEnumerateBoards(_interface);

  // wait until all sensors sent their response. 100 ms timeout
  unsigned int watchdog = 0;
  while (_enumeration_count < getSensorCount() && watchdog < 1e3) {
    watchdog += 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // wait a little longer in case there are more sensors than specified
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  _enumeration_flag = false;
  return _enumeration_count;
}

void SensorBus::requestEEPROM() {
  unsigned int active_devices = 0;
  for (auto& sensor : _sensor_vec) {
    sensor->getThermal()->readEEPROM();
    active_devices |= (sensor->getThermal()->isEnabled() && !sensor->getThermal()->gotEEPROM()) << sensor->getThermal()->getIdx();
  }

  sensor::ThermalSensor::cmdRequestEEPROM(_interface, active_devices);
}

bool SensorBus::allEEPROMTransmissionsComplete() const {
  bool ready = true;

  for (auto& sensor : _sensor_vec) {
    if (sensor->getThermal()->isEnabled()) {
      ready &= sensor->getThermal()->gotEEPROM();
    }
  }

  return ready;
}

void SensorBus::requestTofMeasurement() {
  unsigned int active_devices = 0;
  _active_tof_sensors         = 0;
  _tof_measurement_count      = 0;

  for (auto& sensor : _sensor_vec) {
    if (sensor->getTof()->isEnabled()) {
      active_devices |= 1 << sensor->getTof()->getIdx();
      _active_tof_sensors++;
    }
  }

  sensor::TofSensor::requestTofMeasurement(_interface, active_devices);
}

void SensorBus::fetchTofMeasurement() {
  unsigned int active_devices = 0;
  for (auto& sensor : _sensor_vec) {
    sensor->getTof()->clearDataFlag();
    // check which boards have an active tof sensor
    active_devices |= sensor->getTof()->isEnabled() << sensor->getTof()->getIdx();
  }

  sensor::TofSensor::fetchTofMeasurement(_interface, active_devices);
}

void SensorBus::requestThermalMeasurement() {
  unsigned int active_devices = 0;
  _active_thermal_sensors     = 0;
  _thermal_measurement_count  = 0;

  for (auto& sensor : _sensor_vec) {
    if (sensor->getThermal()->isEnabled()) {
      active_devices |= 1 << sensor->getThermal()->getIdx();
      _active_thermal_sensors++;
    }
  }

  sensor::ThermalSensor::cmdRequestThermalMeasurement(_interface, active_devices);
}

void SensorBus::fetchThermalMeasurement() {
  unsigned int active_devices = 0;
  for (auto& sensor : _sensor_vec) {
    sensor->getThermal()->clearDataFlag();
    active_devices |= sensor->getThermal()->isEnabled() << sensor->getThermal()->getIdx();
  }

  sensor::ThermalSensor::cmdFetchThermalMeasurement(_interface, active_devices);
}

bool SensorBus::allTofMeasurementsReady() const {
  unsigned int unused = 0;
  return allTofMeasurementsReady(unused);
}

bool SensorBus::allTofMeasurementsReady(unsigned int& ready_sensors_count) const {
  ready_sensors_count = 0;
  for (auto& sensor : _sensor_vec) {
    ready_sensors_count += sensor->getTof()->newDataAvailable();
  }
  return _active_tof_sensors == ready_sensors_count;
}

bool SensorBus::allThermalMeasurementsReady() const {
  unsigned int unused = 0;
  return allThermalMeasurementsReady(unused);
}

bool SensorBus::allThermalMeasurementsReady(unsigned int& ready_sensors_count) const {
  // ToDo: Needs actual implementation when more than one thermal sensor is used on one bus
  ready_sensors_count = _thermal_measurement_count;
  return _active_thermal_sensors == _thermal_measurement_count;
}

bool SensorBus::allTofDataTransmissionsComplete() const {
  unsigned int unused = 0;
  return allTofDataTransmissionsComplete(unused);
}

bool SensorBus::allTofDataTransmissionsComplete(unsigned int& ready_sensors_count) const {
  ready_sensors_count = 0;
  for (auto& sensor : _sensor_vec) {
    ready_sensors_count += sensor->getTof()->gotNewData();
  }

  return _active_tof_sensors == ready_sensors_count;
}

bool SensorBus::allThermalDataTransmissionsComplete() const {
  unsigned int unused = 0;
  return allThermalDataTransmissionsComplete(unused);
}

bool SensorBus::allThermalDataTransmissionsComplete(unsigned int& ready_sensors_count) const {
  ready_sensors_count = 0;
  for (auto& sensor : _sensor_vec) {
    ready_sensors_count += sensor->getThermal()->gotNewData();
  }

  return _active_thermal_sensors == ready_sensors_count;
}

bool SensorBus::stopThermalCalibration() {
  bool success = true;

  for (auto& sensor : _sensor_vec) {
    success &= sensor->getThermal()->stopCalibration();
  }

  return success;
}

bool SensorBus::startThermalCalibration(unsigned int window) {
  bool success = true;

  for (auto& sensor : _sensor_vec) {
    success &= sensor->getThermal()->startCalibration(window);
  }

  return success;
}

void SensorBus::notify([[maybe_unused]] const com::ComEndpoint source, [[maybe_unused]] const std::vector<uint8_t>& data) {

  if (source == com::ComEndpoint("broadcast")) { // general sensor board status
    // enumeration message
    if (_enumeration_flag && data.size() == 3 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE) {

      // The bus has to listen to respones to register any boards that are not specified in the configuration
      // Querying the SensorBoards if each has been enumerated can't detect additional boards
      _enumeration_count++;
    }
  }
}
} // namespace bus

} // namespace eduart