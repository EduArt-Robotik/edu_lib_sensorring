#include "sensorring/device/DeviceGroup.hpp"

namespace eduart {

namespace device {

DeviceGroup::DeviceGroup(std::vector<device::BaseDevice*> devices)
    : _devices(devices) {
}

std::vector<device::BaseDevice*> DeviceGroup::getDevices() const {
  return _devices;
}

void DeviceGroup::invokeForEachDevice(std::function<void(device::BaseDevice*)> callback) const {
  for (auto& device : _devices) {
    callback(device);
  }
}

} // namespace device

} // namespace eduart