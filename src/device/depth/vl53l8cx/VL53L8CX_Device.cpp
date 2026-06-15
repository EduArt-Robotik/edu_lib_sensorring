#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Device.hpp"

#include <sensorring_transport/Protocol.hpp>
#include <unordered_map>

#include "device/depth/vl53l8cx/VL53L8CX_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::vl53l8cx;

namespace eduart {

namespace sensorring {

namespace device {

// clang-format off
VL53L8CX_Device::VL53L8CX_Device(VL53L8CX_Params params, com::ComInterfaceID interface, unsigned int idx)
    : DepthSensor( DeviceID({ DeviceType::VL53L8CX, idx}), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::VL53L8CX }, params.enable, { true, true, true }, 45.0, 45.0, 8, 8),
    _impl(std::make_unique<VL53L8CX_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
}
// clang-format on

VL53L8CX_Device::~VL53L8CX_Device() {
}

const VL53L8CX_Params& VL53L8CX_Device::getParams() const {
  return _impl->getParams();
}

void VL53L8CX_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, command, data);
}

std::future<bool> VL53L8CX_Device::requestMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    (void)timeout;
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
      iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::VL53L8CX }, MEASUREMENT_REQUEST, tx_buf);
    }

    for (auto& fut : futures) {
      try {
        if (!fut.get()) {
          return false;
        }
      } catch (const std::future_error&) {
        return false;
      }
    }

    return true;
  });
}

std::future<bool> VL53L8CX_Device::fetchMeasurementAsync(std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [this, timeout]() {
    if (!getEnable()) {
      return false;
    }

    clearDataFlag();
    auto fut = beginMeasurementWait();

    _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::VL53L8CX }, MEASUREMENT_TRANSMISSION_REQUEST, {});

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    if (fut.wait_until(deadline) != std::future_status::ready) {
      return false;
    }
    return fut.get();
  });
}

} // namespace device

} // namespace sensorring

} // namespace eduart
