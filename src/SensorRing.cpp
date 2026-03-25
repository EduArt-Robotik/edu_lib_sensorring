#include "sensorring/SensorRing.hpp"

#include "interface/ComInterface.hpp"

namespace eduart {

namespace ring {

SensorRing::SensorRing(std::vector<std::unique_ptr<bus::SensorBus> > bus_vec)
    : _bus_vec(std::move(bus_vec)) {

  for (const auto& bus : _bus_vec) {
    bus::BusTopology bt;
    bt.interface = bus->getInterface()->getID();

    for (const auto& board : bus->getSensorBoards()) {
      bt.board_type_vec.push_back(board->getBoardType());
    }
    _topology.bus_topology_vec.push_back(std::move(bt));
  }
}

SensorRing::~SensorRing() {
}

std::vector<bus::SensorBus*> SensorRing::getSensorBuses() const {

  std::vector<bus::SensorBus*> ref_vec;
  for (auto& sensor_bus : _bus_vec) {
    ref_vec.push_back(sensor_bus.get());
  }

  return ref_vec;
}

std::vector<device::IDevice*> SensorRing::getDevices() const {
  std::vector<device::IDevice*> devices;
  for (auto& sensor_bus : _bus_vec) {
    for (auto& sensor_board : sensor_bus->getSensorBoards()) {
      devices.push_back(sensor_board);
      for (auto& device : sensor_board->getDevices()) {
        devices.push_back(device);
      }
    }
  }
  return devices;
}

void SensorRing::setBrs(bool brs_enable) {
  for (auto& sensor_bus : _bus_vec) {
    sensor_bus->setBrs(brs_enable);
  }
}

RingTopology SensorRing::getTopology() const noexcept {
  return _topology;
}



} // namespace ring

} // namespace eduart