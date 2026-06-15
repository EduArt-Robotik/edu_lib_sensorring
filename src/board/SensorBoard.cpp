#include "sensorring/board/SensorBoard.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "board/SensorBoardManager.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/interface/ComEndpoint.hpp"

using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace board {

SensorBoard::SensorBoard(SensorBoardParams params, com::ComInterfaceID interface, unsigned int idx, std::vector<std::unique_ptr<device::Device> > devices)
    : _idx(idx)
    , _interface(com::ComManager::getInstance()->getInterface(interface))
    , _params(params)
    , _pose{ params.translation, params.rotation }
    , _enum_info()
    , _device_vec(std::move(devices)) {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
  },
      { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } });
}

SensorBoard::~SensorBoard() {
  // _com_subscription auto-cancels via RAII.
}

bool SensorBoard::isEnumerated() const {
  return _enum_info.type != SensorBoardType::Undefined && _enum_info.hash.hash != 0;
}

const EnumerationInformation& SensorBoard::getEnumInfo() const {
  return _enum_info;
}

SensorBoardType SensorBoard::getBoardType() const {
  return _params.board_type;
}

std::vector<device::Device*> SensorBoard::getDevices() const {
  std::vector<device::Device*> devices;
  devices.reserve(_device_vec.size());
  for (auto& device : _device_vec) {
    devices.push_back(device.get());
  }
  return devices;
}

const Pose& SensorBoard::getPose() const {
  return _pose;
}

void SensorBoard::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  if (command == sensor_board::ACTIVE_DEVICE_RESPONSE && data.size() >= 11 && (data.at(0) == _idx)) {

    RecursiveLock lock(_com_mutex);

    if (_enum_info.isUndefined()) {
      _enum_info       = EnumerationInformation::fromBuffer(data);
      _enum_info.state = ConnectionState::Connected;

      const auto board_type = _enum_info.type;

      // Set pose for all devices on this board in a device-agnostic way.
      for (auto& device : _device_vec) {
        const auto offsets = SensorBoardManager::getDevicePoseOffset(board_type, device->getDeviceID());
        device->setPoseOffset(offsets);
        device->setBoardPose(&_pose);
      }
    }
  }
}

} // namespace board

} // namespace sensorring

} // namespace eduart