#include "board/BoardEnumerator.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "sensorring/board/SensorBoard.hpp"

using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace board {

BoardEnumerator::BoardEnumerator(com::ComInterface* interface)
    : _interface(interface)
    , _enumeration_vec() {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
  },
      { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } });
}

BoardEnumerator::~BoardEnumerator() {
  // _com_subscription auto-cancels via RAII.
}

void BoardEnumerator::startEnumeration() {
  SensorBoard::cmdEnumerateBoards(_interface->getID());
}

std::vector<EnumerationInformation> BoardEnumerator::getResult() {
  LockGuard lock(_enumeration_mutex);
  return _enumeration_vec;
}

void BoardEnumerator::comCallback(const com::ComEndpoint, std::uint8_t command, const std::vector<uint8_t>& data) {
  LockGuard lock(_enumeration_mutex);

  if (command == sensor_board::ACTIVE_DEVICE_RESPONSE) {
    auto info  = EnumerationInformation::fromBuffer(data);
    info.state = ConnectionState::Connected;
    _enumeration_vec.push_back(std::move(info));
  }
}

} // namespace board

} // namespace sensorring

} // namespace eduart