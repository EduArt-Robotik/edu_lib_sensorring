#include "sensorring/device/DeviceGroup.hpp"

namespace eduart {

namespace sensorring {

namespace device {

DeviceGroup::DeviceGroup(std::vector<device::IDevice*> devices)
    : _devices(devices) {
}

std::vector<device::IDevice*> DeviceGroup::getDevices() const {
  return _devices;
}

unsigned int DeviceGroup::getDeviceCount() const {
  return static_cast<unsigned int>(_devices.size());
}

void DeviceGroup::invokeForEachDevice(std::function<void(device::IDevice*)> callback) const {
  for (auto& device : _devices) {
    callback(device);
  }
}

} // namespace device

} // namespace sensorring

} // namespace eduart