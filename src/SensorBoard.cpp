#include "sensorring/SensorBoard.hpp"

#include "device/hardware/SensorBoardManager.hpp"
#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/logger/Logger.hpp"
#include "sensorring/math/Math.hpp"

namespace eduart {

namespace device {

SensorBoard::SensorBoard(SensorBoardParams params, com::ComInterfaceID interface, unsigned int idx, std::vector<std::unique_ptr<BaseDevice> > devices)
    : _idx(idx)
    , _interface(com::ComManager::getInstance()->getInterface(interface))
    , _params(params)
    , _enum_info()
    , _device_vec(std::move(devices)) {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, const std::vector<uint8_t>& data) {
        this->comCallback(source, data);
      },
      { com::ComEndpoint("broadcast") });
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

std::vector<BaseDevice*> SensorBoard::getDevices() const {
  std::vector<BaseDevice*> devices;
  devices.reserve(_device_vec.size());
  for (auto& device : _device_vec) {
    devices.push_back(device.get());
  }
  return devices;
}

void SensorBoard::comCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  if (data.size() == 12 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE && (data.at(1) == _idx)) {

    LockGuard lock(_com_mutex);

    if (_enum_info.isUndefined()) {
      _enum_info       = EnumerationInformation::fromBuffer(data);
      _enum_info.state = ConnectionState::Connected;

      const auto board_type = _enum_info.type;

      // Set pose for all devices on this board in a device-agnostic way.
      for (auto& device : _device_vec) {
        const auto offsets = SensorBoardManager::getDevicePoseOffset(board_type, device->getDeviceID());
        device->setPoseOffset(offsets);

        const auto translation = _params.translation + offsets.board_center_translation_offset;
        const auto rotation    = math::eulerDegreesFromRotationMatrix(math::rotMatrixFromEulerDegrees(_params.rotation) * math::rotMatrixFromEulerDegrees(offsets.board_center_rotation_offset));

        device->setPose(translation, rotation);
      }
    }
  }
}

} // namespace device

} // namespace eduart