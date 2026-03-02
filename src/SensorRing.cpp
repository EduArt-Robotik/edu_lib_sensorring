#include "sensorring/SensorRing.hpp"

#include <cmath>
#include <sstream>

#include "interface/ComManager.hpp"
#include "sensorring/SensorBus.hpp"
#include "sensorring/device/hardware/SensorBoardManager.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
#include "sensorring/logger/Logger.hpp"

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

std::string SensorRing::printTopology() noexcept {
  std::stringstream ss;
  for (const auto& bus : _bus_vec) {
    ss << std::endl << std::endl;
    ss << "=================================================" << std::endl;
    ss << "Topology of the sensors on " << bus->getInterface()->getID().name << ":" << std::endl;
    ss << std::endl;

    auto enum_results = bus->getLatestEnumerationResult();
    for (const auto& device : enum_results) {

      ss << "sensor " << device.idx << std::endl;
      ss << "    Type:           " << device.type << std::endl;
      ss << "    State:          " << device.state << std::endl;
      ss << "    FW revision:    " << device.version << " (" << device.hash << ")" << std::endl;

      for (const auto& dev : device.devices) {
        ss << "    Device:         " << dev;
        ss << std::endl;
      }

      ss << std::endl;
    }

    ss << "=================================================" << std::endl;
  }
  return ss.str();
}

RingTopology SensorRing::getTopology() const noexcept {
  return _topology;
}

bool SensorRing::verifyTopology() const {
  return true;
}

std::unique_ptr<SensorRing> SensorRing::createFromEnumeration(std::vector<com::ComInterfaceID> interfaces) {
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Creating a SensorRing from enumeration.");

  std::vector<std::unique_ptr<bus::SensorBus> > bus_vec;

  for (const auto& interface : interfaces) {
    auto iface = com::ComManager::getInstance()->getInterface(interface);
    if (!iface) {
      continue;
    }

    auto id = iface->getID(); // If the iface was automatically generated (e.g. USBTINGO & Serial 0) the actual id is different from the one passed to this method -> have to fetch the actual one

    auto enum_infos = bus::SensorBus::queryConnectedDevices(id);

    if (!enum_infos.empty()) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Found " + std::to_string(enum_infos.size()) + " sensor boards on interface " + id.name + " during creation.");

      std::vector<std::unique_ptr<device::SensorBoard> > board_vec;
      board_vec.reserve(enum_infos.size());

      for (const auto& enum_info : enum_infos) {
        unsigned int idx = (enum_info.idx > 0u) ? enum_info.idx - 1u : 0u;
        device::SensorBoardParams board_params;
        board_params.board_type = enum_info.type;
        board_vec.push_back(device::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx));
      }
      bus_vec.push_back(std::make_unique<bus::SensorBus>(id, std::move(board_vec)));
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "Found no sensor boards on interface " + id.name + " during creation. Skipping this interface.");
    }
  }

  if (bus_vec.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Found no sensor boards on any of the provided interfaces. Failed to create SensorRing.");
    return nullptr;
  }

  return std::make_unique<SensorRing>(std::move(bus_vec));
}

} // namespace ring

} // namespace eduart