#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"

#include <unordered_map>

#include "device/hardware/vl53l8cx/VL53L8CX_DeviceImpl.hpp"
#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace device {

VL53L8CX_Device::VL53L8CX_Device(VL53L8CX_Params params, com::ComInterfaceID interface, unsigned int idx)
    : BaseDevice(DeviceID({ DeviceType::VL53L8CX, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint("tof" + std::to_string(idx) + "_data"), params.enable)
    , _impl(std::make_unique<VL53L8CX_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
}

VL53L8CX_Device::~VL53L8CX_Device() {
}

const VL53L8CX_Params& VL53L8CX_Device::getParams() const {
  return _impl->getParams();
}

std::pair<const measurement::TofMeasurement&, DeviceState> VL53L8CX_Device::getLatestMeasurement() const {
  return _impl->getLatestMeasurement();
}

std::pair<const measurement::TofMeasurement&, DeviceState> VL53L8CX_Device::getLatestTransformedMeasurement() const {
  return _impl->getLatestTransformedMeasurement();
}

void VL53L8CX_Device::onResetSensorState() {
  _impl->onResetSensorState();
}

void VL53L8CX_Device::onClearDataFlag() {
  _impl->onClearDataFlag();
}

void VL53L8CX_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, data);
}

// std::future<bool> VL53L8CX_Device::requestTofMeasurementAsync(std::chrono::milliseconds timeout) {
//   return std::async(std::launch::async, [this, timeout]() {
//     auto fut = beginDataAvailableWait();

//     if (!getEnable()) {
//       setDataAvailableReady(false);
//       return false;
//     }

//     static std::atomic<std::uint8_t> request_count{ 0 };
//     std::uint8_t count          = request_count.fetch_add(1, std::memory_order_relaxed);
//     unsigned int active_sensors = (1u << static_cast<unsigned int>(getIdx()));
//     uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
//     uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
//     std::vector<uint8_t> tx_buf = { count, sensor_select_high, sensor_select_low };
//     _interface->send(com::ComEndpoint("tof_request"), tx_buf);

//     const auto deadline = std::chrono::steady_clock::now() + timeout;
//     if (fut.wait_until(deadline) != std::future_status::ready) {
//       return false;
//     }
//     return fut.get();
//   });
// }

// std::future<bool> VL53L8CX_Device::fetchTofMeasurementAsync(std::chrono::milliseconds timeout) {
//   return std::async(std::launch::async, [this, timeout]() {
//     auto fut = beginMeasurementWait();

//     if (!getEnable()) {
//       setMeasurementReady(false);
//       return false;
//     }

//     clearDataFlag();
//     unsigned int active_sensors = (1u << static_cast<unsigned int>(getIdx()));
//     uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
//     uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
//     std::vector<uint8_t> tx_buf = { sensor_select_high, sensor_select_low };
//     _interface->send(com::ComEndpoint("tof_request"), tx_buf);

//     const auto deadline = std::chrono::steady_clock::now() + timeout;
//     if (fut.wait_until(deadline) != std::future_status::ready) {
//       return false;
//     }
//     return fut.get();
//   });
// }

std::future<bool> VL53L8CX_Device::requestTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    struct InterfaceGroup {
      std::vector<VL53L8CX_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;
    std::vector<std::future<bool> > futures;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);
        futures.emplace_back(dev->beginDataAvailableWait());
        group.active_sensors |= (1u << dev->getIdx());
      }
    }

    if (groups.empty()) {
      return false;
    }

    static std::atomic<std::uint8_t> request_count{ 0 };
    std::uint8_t count = request_count.fetch_add(1, std::memory_order_relaxed);

    for (auto& [iface, group] : groups) {
      if (group.devices.empty()) {
        continue;
      }
      unsigned int active_sensors = group.active_sensors;
      uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
      uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
      std::vector<uint8_t> tx_buf{ count, sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint("tof_request"), tx_buf);
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;

    for (auto& fut : futures) {
      if (fut.wait_until(deadline) != std::future_status::ready) {
        return false;
      }
      if (!fut.get()) {
        return false;
      }
    }

    return true;
  });
}

std::future<bool> VL53L8CX_Device::fetchTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    struct InterfaceGroup {
      std::vector<VL53L8CX_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;
    std::vector<std::future<bool> > futures;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);

        // Clear flags for the new cycle first, then start a fresh wait future.
        dev->clearDataFlag();
        futures.emplace_back(dev->beginMeasurementWait());

        group.active_sensors |= (1u << dev->getIdx());
      }
    }

    if (groups.empty()) {
      return false;
    }

    // Send one fetch per interface.
    for (auto& [iface, group] : groups) {
      if (group.devices.empty()) {
        continue;
      }
      unsigned int active_sensors = group.active_sensors;
      uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
      uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
      std::vector<uint8_t> tx_buf{ sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint("tof_request"), tx_buf);
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;

    // Wait until all enabled devices (across all interfaces) have got a new measurement.
    for (auto& fut : futures) {
      if (fut.wait_until(deadline) != std::future_status::ready) {
        return false;
      }
      if (!fut.get()) {
        return false;
      }
    }

    return true;
  });
}

} // namespace device

} // namespace eduart
