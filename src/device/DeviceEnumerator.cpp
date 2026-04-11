#include "DeviceEnumerator.hpp"

#include <sensorring_transport/Protocol.hpp>

using namespace eduart::transport::protocol;

#include "SensorBoardCommands.hpp"

namespace eduart {

namespace sensorring {

namespace device {

DeviceEnumerator::DeviceEnumerator(com::ComInterface* interface)
    : _interface(interface)
    , _enumeration_vec() {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
  },
      { com::ComEndpoint{ com::Direction::Input, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } });
}

DeviceEnumerator::~DeviceEnumerator() {
  // _com_subscription auto-cancels via RAII.
}

void DeviceEnumerator::startEnumeration() {
  device::cmdEnumerateBoards(_interface->getID());
}

std::vector<device::EnumerationInformation> DeviceEnumerator::getResult() {
  LockGuard lock(_enumeration_mutex);
  return _enumeration_vec;
}

void DeviceEnumerator::comCallback(const com::ComEndpoint, std::uint8_t command, const std::vector<uint8_t>& data) {
  LockGuard lock(_enumeration_mutex);

  if (command == sensor_board::ACTIVE_DEVICE_RESPONSE && data.size() >= 11) {
    auto info  = device::EnumerationInformation::fromBuffer(data);
    info.state = device::ConnectionState::Connected;
    _enumeration_vec.push_back(std::move(info));
  }
}

} // namespace device

} // namespace sensorring

} // namespace eduart