#include "sensorring/device/Device.hpp"

#include "interface/ComInterface.hpp"

namespace eduart {

namespace sensorring {

namespace device {

Device::Device(DeviceID id, com::ComInterface* interface, com::ComEndpoint target)
    : _id(id)
    , _hw_idx(id.getIndex())
    , _interface(interface) {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
      },
      { target });
}

Device::~Device() {
  // _com_subscription auto-cancels via RAII.
}

DeviceID Device::getDeviceID() const {
  return _id;
}

unsigned int Device::getHwIdx() const {
  return _hw_idx;
}

void Device::setDeviceIndex(unsigned int index) {
  _id.index = index;
}

bool Device::configure() {
  return true;
}

void Device::setBoardContext(const board::SensorBoardParams* board_params) {
  _board_params = board_params;
}

const board::SensorBoardParams* Device::getBoardContext() const {
  return _board_params;
}

void Device::setPoseOffset(const math::Pose& offset) {
  _offset = offset;
}

math::Pose Device::getPoseOffset() const {
  return _offset;
}

math::Pose Device::getGlobalPose() const {
  if (_board_params) {
    const math::Pose board_pose{ _board_params->translation, _board_params->rotation };
    return board_pose + _offset;
  }
  return _offset;
}

} // namespace device

} // namespace sensorring

} // namespace eduart