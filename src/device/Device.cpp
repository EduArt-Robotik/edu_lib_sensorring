#include "sensorring/device/Device.hpp"

#include "interface/ComInterface.hpp"

namespace eduart {

namespace sensorring {

namespace device {

Device::Device(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable)
    : _id(id)
    , _hw_idx(id.getIndex())
    , _enable(enable)
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

void Device::enqueueAction(std::function<void()> action) {
  std::lock_guard<std::mutex> lock(_action_mutex);
  _pending_actions.push_back(std::move(action));
}

void Device::setReplacableAction(std::function<void()> action) {
  std::lock_guard<std::mutex> lock(_action_mutex);
  _replaceable_action = std::move(action);
}

std::vector<std::function<void()> > Device::drainActions() {
  std::lock_guard<std::mutex> lock(_action_mutex);
  std::vector<std::function<void()> > actions;
  actions.swap(_pending_actions);
  if (_replaceable_action) {
    actions.push_back(std::move(*_replaceable_action));
    _replaceable_action.reset();
  }
  return actions;
}

DeviceID Device::getDeviceID() const {
  return _id;
}

unsigned int Device::getIdx() const {
  return _hw_idx;
}

void Device::setDeviceIndex(unsigned int index) {
  _id.index = index;
}

void Device::setEnable(bool enable) {
  _enable = enable;
}

bool Device::getEnable() const {
  return _enable;
}

bool Device::configure() {
  return true;
}

void Device::setPose(Pose pose) {
  _pose = pose;
  _rot_m       = math::rotMatrixFromEulerDegrees(_pose.orientation);
}

Pose Device::getPose() const {
  return _pose;
}

void Device::setPoseOffset(const Pose& offset) {
  _offset = offset;
}

Pose Device::getPoseOffset() const {
  return _offset;
}

} // namespace device

} // namespace sensorring

} // namespace eduart