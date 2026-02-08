#include "sensorring/device/BaseDevice.hpp"

namespace eduart {

namespace device {

BaseDevice::BaseDevice(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable)
    : BaseSensor(interface, target, id.getIndex(), enable)
    , _id(id)
    , _enable(false) {
}

DeviceID BaseDevice::getDeviceID() const {
  return _id;
}

// void BaseDevice::setEnable(bool enable) {
//   _enable = enable;
// }

// bool BaseDevice::getEnable() const {
//   return _enable;
// }

} // namespace device

} // namespace eduart