#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "device/hardware/htpa32/HTPA32_DeviceImpl.hpp"

#include <unordered_map>

#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace device {

HTPA32_Device::HTPA32_Device(HTPA32_Params params, com::ComInterface* interface, unsigned int idx)
    : BaseDevice(DeviceID({ DeviceType::HTPA32, "thermal", idx }),
                 interface,
                 com::ComEndpoint("thermal" + std::to_string(idx) + "_data"),
                 params.enable)
    , _impl(std::make_unique<HTPA32_DeviceImpl>(*this, params, interface, idx)) {}

HTPA32_Device::~HTPA32_Device() {
}

HTPA32_Params HTPA32_Device::getParams() const { return _impl->getParams(); }

std::pair<const measurement::GrayscaleImage&, SensorState> HTPA32_Device::getLatestGrayscaleImage() const {
  return _impl->getLatestGrayscaleImage();
}

std::pair<const measurement::FalseColorImage&, SensorState> HTPA32_Device::getLatestFalseColorImage() const {
  return _impl->getLatestFalseColorImage();
}

std::pair<const measurement::ThermalMeasurement&, SensorState> HTPA32_Device::getLatestMeasurement() const {
  return _impl->getLatestMeasurement();
}

bool HTPA32_Device::stopCalibration() {
  return _impl->stopCalibration();
}

bool HTPA32_Device::startCalibration(std::size_t window) {
  return _impl->startCalibration(window);
}

void HTPA32_Device::onResetSensorState() {
  _impl->onResetSensorState();
}

void HTPA32_Device::onClearDataFlag() {
  _impl->onClearDataFlag();
}

void HTPA32_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, data);
}

std::future<bool> HTPA32_Device::getEpromAsync(std::chrono::milliseconds timeout) {
  return _impl->getEpromAsync(timeout);
}

// std::future<bool> HTPA32_Device::requestThermalMeasurementAsync(std::chrono::milliseconds /*timeout*/) {
//   return std::async(std::launch::async, [this]() {
//     if (!getEnable())
//       return false;

//     unsigned int active_sensors = (1u << static_cast<unsigned int>(getIdx()));
//     uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
//     uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
//     std::vector<uint8_t> tx_buf = { CMD_THERMAL_SCAN_REQUEST, sensor_select_high, sensor_select_low };
//     _interface->send(com::ComEndpoint("thermal_request"), tx_buf);

//     // Fire-and-forget: MeasurementManager tracks timing and will wait on fetch futures.
//     return true;
//   });
//}

// std::future<bool> HTPA32_Device::fetchThermalMeasurementAsync(std::chrono::milliseconds timeout) {
//   // Single-device helper kept for completeness; the group overload is used by the manager.
//   return std::async(std::launch::async, [this, timeout]() {
//     // Prepare a per-cycle future that will be fulfilled from the callback thread.
//     auto fut = beginMeasurementWait();

//     if (!getEnable()) {
//       setMeasurementReady(false);
//       return false;
//     }

//     clearDataFlag();
//     unsigned int active_sensors = (1u << static_cast<unsigned int>(getIdx()));
//     uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
//     uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
//     std::vector<uint8_t> tx_buf = { CMD_THERMAL_DATA_REQUEST, sensor_select_high, sensor_select_low };
//     _interface->send(com::ComEndpoint("thermal_request"), tx_buf);

//     const auto deadline = std::chrono::steady_clock::now() + timeout;
//     if (fut.wait_until(deadline) != std::future_status::ready) {
//       return false;
//     }
//     return fut.get();
//   });
// }

std::future<bool> HTPA32_Device::requestThermalMeasurementAsync(const std::vector<HTPA32_Device*>& devices, std::chrono::milliseconds /*timeout*/) {
  return std::async(std::launch::async, [devices]() {
    // Group enabled devices by their communication interface, clear flags and
    // build per-interface masks in a single pass.
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
        group.active_sensors |= (1u << static_cast<unsigned int>(dev->getIdx()));
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
      std::vector<uint8_t> tx_buf{ CMD_THERMAL_SCAN_REQUEST, sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint("thermal_request"), tx_buf);
    }

    // Fire-and-forget: success means the request was issued for all enabled devices.
    return true;
  });
}

std::future<bool> HTPA32_Device::fetchThermalMeasurementAsync(const std::vector<HTPA32_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    // Group enabled devices by their communication interface, clear flags and
    // build per-interface masks in a single pass. For each device, start a
    // fresh measurement-wait cycle backed by a std::promise/std::future pair.
    struct InterfaceGroup {
      std::vector<HTPA32_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;
    std::vector<std::future<bool> > futures;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);

        // Clear data flags for this measurement cycle first, then start a new wait future.
        dev->clearDataFlag();
        futures.emplace_back(dev->beginMeasurementWait());

        group.active_sensors |= (1u << static_cast<unsigned int>(dev->getIdx()));
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
      std::vector<uint8_t> tx_buf{ CMD_THERMAL_DATA_REQUEST, sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint("thermal_request"), tx_buf);
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;

    // Wait until all enabled devices (across all interfaces) have produced a
    // new measurement and reported success through their futures.
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
