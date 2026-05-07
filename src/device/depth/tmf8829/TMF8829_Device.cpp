#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "device/depth/tmf8829/TMF8829_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::tmf8829;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_Device::TMF8829_Device(TMF8829_Params params, com::ComInterfaceID interface, unsigned int idx)
    : BaseDevice(DeviceID({ DeviceType::TMF8829, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::TMF8829 }, params.enable)
    , _impl(std::make_unique<TMF8829_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
}

TMF8829_Device::~TMF8829_Device() {
}

const TMF8829_Params& TMF8829_Device::getParams() const {
  return _impl->getParams();
}

std::pair<const measurement::DepthMeasurement&, DeviceState> TMF8829_Device::getLatestMeasurement() const {
  return _impl->getLatestMeasurement();
}

std::pair<const measurement::DepthMeasurement&, DeviceState> TMF8829_Device::getLatestTransformedMeasurement() const {
  return _impl->getLatestTransformedMeasurement();
}

void TMF8829_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, command, data);
}

void TMF8829_Device::publishMeasurement() {
  if (!getEnable())
    return;
  auto [raw, state]          = _impl->getLatestMeasurement();
  auto [transformed, state2] = _impl->getLatestTransformedMeasurement();

  measurement::DepthMeasurement m;
  m.sensor_index            = _id.index;
  m.frame_id                = raw.frame_id;
  m.nr_valid_points         = raw.nr_valid_points;
  m.timestamp               = std::chrono::system_clock::now();
  m.state                   = state;
  m.point_cloud             = raw.point_cloud;
  m.transformed_point_cloud = transformed.point_cloud;
  _depth_publisher.publish(m);
}

std::future<bool> TMF8829_Device::requestMeasurementAsync(const std::vector<TMF8829_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    struct InterfaceGroup {
      std::vector<TMF8829_Device*> devices;
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
      iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::TMF8829 }, MEASUREMENT_REQUEST, tx_buf);
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

std::future<bool> TMF8829_Device::fetchMeasurementAsync(std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [this, timeout]() {
    if (!getEnable()) {
      return false;
    }

    clearDataFlag();
    auto fut = beginMeasurementWait();

    _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_idx + 1), devbyte::TMF8829 }, MEASUREMENT_TRANSMISSION_REQUEST, {});

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
