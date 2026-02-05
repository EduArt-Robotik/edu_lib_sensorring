#include "sensorring/device/IDevice.hpp"

namespace eduart {

namespace device {

const DeviceID& IDevice::getID() const {
  return _id;
}

void IDevice::setID(const DeviceID& id) {
  _id = id;
}

} // namespace device

} // namespace eduart