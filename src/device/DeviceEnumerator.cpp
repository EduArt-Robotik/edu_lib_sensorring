#include "DeviceEnumerator.hpp"

#include "sensorring/SensorBoard.hpp"
#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace device {

DeviceEnumerator::DeviceEnumerator(com::ComInterface* interface)
    : _interface(interface)
    , _enumeration_vec() {
  subscribeToEndpoint(com::ComEndpoint("broadcast"));
  _interface->registerObserver(this);
}

DeviceEnumerator::~DeviceEnumerator() {
  _interface->unregisterObserver(this);
}

void DeviceEnumerator::startEnumeration() {
  device::SensorBoard::cmdEnumerateBoards(_interface->getID());
}

std::vector<device::EnumerationInformation> DeviceEnumerator::getResult() {
  LockGuard lock(_enumeration_mutex);
  return _enumeration_vec;
}

void DeviceEnumerator::comCallback(const com::ComEndpoint, const std::vector<uint8_t>& data) {
  LockGuard lock(_enumeration_mutex);

  if (data.size() == 12 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE) {
    auto info  = device::EnumerationInformation::fromBuffer(data);
    info.state = device::ConnectionState::Connected;
    _enumeration_vec.push_back(std::move(info));
  }
}

} // namespace device

} // namespace eduart