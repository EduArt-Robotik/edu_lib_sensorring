#include "sensorring/SensorRing.hpp"

#include <chrono>
#include <cmath>
#include <sstream>

#include "boardmanager/SensorBoardManager.hpp"
#include "interface/ComManager.hpp"
#include "SensorBus.hpp"
#include "types/EnumerationInformation.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
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

std::string SensorRing::printTopology() const noexcept {
  std::stringstream ss;
  for (const auto& bus : getInterfaces()) {
    ss << std::endl << std::endl;
    ss << "=================================================" << std::endl;
    ss << "Topology of the sensors on " << bus->getInterface()->getInterfaceName() << ":" << std::endl;
    ss << std::endl;

    auto enum_info_vec = bus->getEnumerationInfo();
    for (const auto& enum_info : enum_info_vec) {
      const auto& board_infos = device::SensorBoardManager::getSensorBoardInfo(enum_info.type);

      ss << "sensor " << enum_info.idx << std::endl;
      ss << "    Type:           " << board_infos.name << std::endl;
      ss << "    State:          " << device::toString(enum_info.state) << std::endl;
      ss << "    FW revision:    " << enum_info.version << " (" << enum_info.hash << ")" << std::endl;

      for (const auto& dev : board_infos.devices) {
        std::string device_info_name = "unknown";

        std::visit(
            [&](const auto& info) {
              using T = std::decay_t<decltype(info)>;
              if constexpr (std::is_same_v<T, std::monostate>) {
                device_info_name = "unknown";
              } else {
                device_info_name = std::string(info.name);
              }
            },
            dev.info);

        ss << "    Device:         " << dev.id.name;
        ss << " (" << device_info_name << ")";
        ss << std::endl;
      }

      ss << std::endl;
    }

    ss << "=================================================" << std::endl;
  }
  return ss.str();
}

std::unique_ptr<SensorRing> SensorRing::create(RingParams params) {
  std::vector<std::unique_ptr<bus::SensorBus> > bus_vec;
  for (const auto& bus_params : params.bus_param_vec) {
    auto interface = com::ComManager::getInstance()->createInterface(bus_params.interface_name, bus_params.type);

    unsigned int idx = 0;
    std::vector<std::unique_ptr<device::SensorBoard> > board_vec;
    for (const auto& board_params : bus_params.board_param_vec) {
      std::vector<std::unique_ptr<device::BaseDevice> > devices;
      devices.push_back(std::make_unique<device::VL53L8CX_Device>(board_params.vl53l8cx_params, interface, idx));
      devices.push_back(std::make_unique<device::HTPA32_Device>(board_params.htpa32_params, interface, idx));
      devices.push_back(std::make_unique<device::WS2812b_Device>(board_params.ws2812b_params, interface));

      board_vec.push_back(std::make_unique<device::SensorBoard>(board_params, interface, idx, std::move(devices)));
      idx++;
    }

    bus_vec.push_back(std::make_unique<bus::SensorBus>(interface, std::move(board_vec)));
  }
  return std::make_unique<SensorRing>(params, std::move(bus_vec));
}

} // namespace ring

} // namespace eduart