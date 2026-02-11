#include "sensorring/device/DeviceGroup.hpp"

namespace eduart {

namespace device {

DeviceGroup::DeviceGroup(std::vector<device::IDevice*> devices)
    : _devices(devices) {
}

std::vector<device::IDevice*> DeviceGroup::getDevices() const {
  return _devices;
}

std::size_t DeviceGroup::getDeviceCount() const {
  return _devices.size();
}

void DeviceGroup::invokeForEachDevice(std::function<void(device::IDevice*)> callback) const {
  for (auto& device : _devices) {
    callback(device);
  }
}

} // namespace device

} // namespace eduart