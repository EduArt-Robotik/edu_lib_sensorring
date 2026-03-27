#include "DeviceEnumerator.hpp"

#include "interface/can/canprotocol.hpp"
#include "sensorring/SensorBoard.hpp"

namespace eduart {

namespace device {

DeviceEnumerator::DeviceEnumerator(com::ComInterface* interface)
    : _interface(interface)
    , _enumeration_vec() {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, const std::vector<uint8_t>& data) {
        this->comCallback(source, data);
      },
      { com::ComEndpoint("broadcast") });
}

DeviceEnumerator::~DeviceEnumerator() {
  // _com_subscription auto-cancels via RAII.
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