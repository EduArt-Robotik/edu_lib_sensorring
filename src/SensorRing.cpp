#include "SensorRing.hpp"

#include <cmath>
#include <memory>
#include <chrono>

#include "sensorring/logger/Logger.hpp"

using namespace std::chrono_literals;

namespace eduart {

namespace ring {

SensorRing::SensorRing(RingParams params, std::vector<std::unique_ptr<bus::SensorBus> > bus_vec)
    : _params(params)
    , _bus_vec(std::move(bus_vec)) {

  if (_params.timeout == 0ms) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "SensorRing timeout parameter is 0.0s");
  } else if (_params.timeout < 200ms) {
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

void SensorRing::resetSensorState() {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->resetSensorState();
  }
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

void SensorRing::setBrs(bool brs_enable) {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->setBrs(brs_enable);
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

  // wait until all sensors sent their response. Timeout protected
  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  do {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allEEPROMTransmissionsComplete();
    }
    if (!ready) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  } while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout);

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

  do {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allTofMeasurementsReady();
    }
    if (!ready) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
  } while (!ready && ((std::chrono::steady_clock::now() - timestamp) < _params.timeout));

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

  do {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allTofDataTransmissionsComplete();
    }
    if (!ready) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
  } while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout);

  return ready;
}

bool SensorRing::waitForAllThermalDataTransmissionsComplete() const {
  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  do {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allThermalDataTransmissionsComplete();
    }
    if (!ready) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
  } while (!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout);

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

bool SensorRing::startThermalCalibration(std::size_t window) {
  bool success = true;

  for (auto& sensor_bus : _bus_vec) {
    success &= sensor_bus->startThermalCalibration(window);
  }

  return success;
}

} // namespace ring

} // namespace eduart