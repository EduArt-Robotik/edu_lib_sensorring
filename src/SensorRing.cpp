#include "SensorRing.hpp"

#include <cmath>
#include <memory>

#include "logger/Logger.hpp"

namespace eduart {

namespace ring {

SensorRing::SensorRing(RingParams params, std::vector<std::unique_ptr<bus::SensorBus> > bus_vec)
    : _params(params)
    , _bus_vec(std::move(bus_vec)) {

  if (_params.timeout == std::chrono::milliseconds(0)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter is 0.0s");
  } else if (_params.timeout < std::chrono::milliseconds(200)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter of " + std::to_string(_params.timeout.count()) + " ms is probably too low");
  }
}

SensorRing::~SensorRing() {
}

std::vector<const bus::SensorBus*> SensorRing::getInterfaces() const {

  std::vector<const bus::SensorBus*> ref_vec;
  for (const auto& sensor_bus : _bus_vec) {
    ref_vec.push_back(sensor_bus.get());
  }

  return ref_vec;
}

void SensorRing::resetDevices() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->resetDevices();
  }
}

bool SensorRing::enumerateDevices() {
  size_t sensor_count = 0;
  bool success        = true;

  for (auto& sensor_bus : _bus_vec) {
    sensor_count = sensor_bus->enumerateDevices();
    success &= (sensor_bus->getSensorCount() == sensor_count);
  }

  return success;
}

void SensorRing::setBRS(bool brs_enable) {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->setBRS(brs_enable);
  }
}

void SensorRing::syncLight() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->syncLight();
  }
}

void SensorRing::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->setLight(mode, red, green, blue);
  }
}

bool SensorRing::getEEPROM() {

  // request transmission of eeprom from all thermal sensors
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->requestEEPROM();
  }

  // wait until all sensors sent their response. 1 sec timeout
  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout) {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allEEPROMTransmissionsComplete();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  return ready;
}

void SensorRing::requestTofMeasurement() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->requestTofMeasurement();
  }
}

bool SensorRing::waitForAllTofMeasurementsReady() const {

  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout) {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allTofMeasurementsReady();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  return ready;
}

bool SensorRing::waitForAllThermalMeasurementsReady() const {
  // int watchdog = 0;
  // bool ready = false;

  // while(!ready && watchdog < _params.timeout){
  // 	ready = true;
  // 	for(auto& sensor_bus : _bus_vec){
  // 		ready &= sensor_bus->allThermalMeasurementsReady();
  // 	}

  // 	watchdog += 100;
  // 	std::this_thread::sleep_for(std::chrono::microseconds(1);
  // }

  // return ready; // ToDo: Check if a thermal frame is actually available
  return true;
}

bool SensorRing::waitForAllTofDataTransmissionsComplete() const {
  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout) {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allTofDataTransmissionsComplete();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  return ready;
}

bool SensorRing::waitForAllThermalDataTransmissionsComplete() const {
  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout) {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allThermalDataTransmissionsComplete();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  return ready;
}

void SensorRing::fetchTofMeasurement() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->fetchTofMeasurement();
  }
}

void SensorRing::requestThermalMeasurement() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->requestThermalMeasurement();
  }
}

void SensorRing::fetchThermalMeasurement() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->fetchThermalMeasurement();
  }
}

bool SensorRing::stopThermalCalibration() {
  bool success = true;

  for (auto& sensor_bus : _bus_vec) {
    success &= sensor_bus->stopThermalCalibration();
  }

  return success;
}

bool SensorRing::startThermalCalibration(size_t window) {
  bool success = true;

  for (auto& sensor_bus : _bus_vec) {
    success &= sensor_bus->startThermalCalibration(window);
  }

  return success;
}

} // namespace ring

} // namespace eduart