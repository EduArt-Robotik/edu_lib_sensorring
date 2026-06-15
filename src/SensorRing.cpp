#include "sensorring/SensorRing.hpp"

namespace eduart {

namespace sensorring {

SensorRing::SensorRing(std::vector<std::unique_ptr<SensorBus> > bus_vec) {
  _bus_vec.reserve(bus_vec.size());
  for (auto& bus : bus_vec) {
    _bus_vec.push_back(std::move(bus));
  }
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

std::vector<device::Device*> SensorRing::getDevices() const {
  std::vector<device::Device*> devices;
  for (auto& sensor_bus : _bus_vec) {
    for (auto& sensor_board : sensor_bus->getSensorBoards()) {
      for (auto& device : sensor_board->getDevices()) {
        devices.push_back(device);
      }
    }
  }
  return devices;
}

} // namespace sensorring

} // namespace eduart