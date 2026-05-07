#include "sensorring/SensorRing.hpp"

namespace eduart {

namespace sensorring {

namespace ring {

SensorRing::SensorRing(std::vector<std::unique_ptr<SensorBus> > bus_vec)
    : _bus_vec(std::move(bus_vec)) {
}

SensorRing::~SensorRing() {
}

std::vector<SensorBus*> SensorRing::getSensorBuses() const {

  std::vector<SensorBus*> ref_vec;
  for (auto& sensor_bus : _bus_vec) {
    ref_vec.push_back(sensor_bus.get());
  }

  return ref_vec;
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

void SensorRing::setBitRateSwitching(bool brs_enable) {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->setBitRateSwitching(brs_enable);
  }
}

} // namespace ring

} // namespace sensorring

} // namespace eduart