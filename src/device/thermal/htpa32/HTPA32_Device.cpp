#include "sensorring/device/thermal/htpa32/HTPA32_Device.hpp"

#include <sensorring_transport/Protocol.hpp>
#include <unordered_map>

#include "device/thermal/htpa32/HTPA32_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::htpa32;

namespace eduart {

namespace sensorring {

namespace device {

HTPA32_Device::HTPA32_Device(HTPA32_Params params, com::ComInterfaceID interface, unsigned int idx)
    : ThermalSensor(DeviceID({ DeviceType::HTPA32, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::HTPA32 }, params.enable)
    , _impl(std::make_unique<HTPA32_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
}

HTPA32_Device::~HTPA32_Device() {
}

const HTPA32_Params& HTPA32_Device::getParams() const {
  return _impl->getParams();
}

bool HTPA32_Device::stopCalibration() {
  return _impl->stopCalibration();
}

bool HTPA32_Device::startCalibration(unsigned int window) {
  return _impl->startCalibration(window);
}

void HTPA32_Device::onClearDataFlag() {
  _impl->onClearDataFlag();
}

void HTPA32_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, command, data);
}

std::future<bool> HTPA32_Device::getEepromAsync(std::chrono::milliseconds timeout) {
  return _impl->getEepromAsync(timeout);
}

std::future<bool> HTPA32_Device::requestMeasurementAsync(const std::vector<HTPA32_Device*>& devices, std::chrono::milliseconds /*timeout*/) {
  return std::async(std::launch::async, [devices]() {
    struct InterfaceGroup {
      std::vector<HTPA32_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);
        group.active_sensors |= (1u << dev->getHwIdx());
      }
    }

    if (groups.empty()) {
      return false;
    }

    for (auto& [iface, group] : groups) {
      if (group.devices.empty()) {
        continue;
      }
      unsigned int active_sensors = group.active_sensors;
      uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
      uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
      std::vector<uint8_t> tx_buf{ sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::HTPA32 }, MEASUREMENT_REQUEST, tx_buf);
    }

    // Fire-and-forget: success means the request was issued for all enabled devices.
    return true;
  });
}

std::future<bool> HTPA32_Device::requestMeasurementAsync(std::chrono::milliseconds /*timeout*/) {
  return std::async(std::launch::async, [this]() {
    if (!getEnable()) {
      return false;
    }

    _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::HTPA32 }, MEASUREMENT_REQUEST, {});

    return true;
  });
}

std::future<bool> HTPA32_Device::fetchMeasurementAsync(std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [this, timeout]() {
    if (!getEnable()) {
      return false;
    }

    clearDataFlag();
    auto fut = beginMeasurementWait();

    _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::HTPA32 }, MEASUREMENT_TRANSMISSION_REQUEST, {});

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
