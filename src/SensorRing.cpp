#include "sensorring/SensorRing.hpp"

#include <chrono>
#include <cmath>
#include <memory>

#include "interface/ComManager.hpp"
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

std::vector<device::BaseDevice*> SensorRing::getDevices() const {
  std::vector<device::BaseDevice*> devices;
  for (auto& sensor_bus : _bus_vec) {
    for (auto& sensor_board : sensor_bus->getSensorBoards()) {
      for (auto& device : sensor_board->getDevices()) {
        devices.push_back(device);
      }
    }
  }
  return devices;
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
  bool ready     = false;
  auto timestamp = std::chrono::steady_clock::now();

  do {
    ready = true;
    for (auto& sensor_bus : _bus_vec) {
      ready &= sensor_bus->allThermalMeasurementsReady();
    }
    if (!ready) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
  } while (!ready && ((std::chrono::steady_clock::now() - timestamp) < _params.timeout));

  return ready;
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

std::unique_ptr<SensorRing> SensorRing::create(RingParams params) {
  std::vector<std::unique_ptr<bus::SensorBus> > bus_vec;
  for (const auto& bus_params : params.bus_param_vec) {
    auto interface = com::ComManager::getInstance()->createInterface(bus_params.interface_name, bus_params.type);

    unsigned int idx = 0;
    std::vector<std::unique_ptr<device::SensorBoard> > board_vec;
    for (const auto& board_params : bus_params.board_param_vec) {
      auto tof     = std::make_unique<device::TofSensor>(board_params.tof_params, interface, idx);
      auto thermal = std::make_unique<device::ThermalSensor>(board_params.thermal_params, interface, idx);
      auto light   = std::make_unique<device::LedLight>(board_params.light_params, interface);

      board_vec.push_back(std::make_unique<device::SensorBoard>(board_params, interface, idx, std::move(tof), std::move(thermal), std::move(light)));
      idx++;
    }

    bus_vec.push_back(std::make_unique<bus::SensorBus>(interface, std::move(board_vec)));
  }
  return std::make_unique<SensorRing>(params, std::move(bus_vec));
}

} // namespace ring

} // namespace eduart