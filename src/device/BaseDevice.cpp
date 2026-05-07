#include "sensorring/device/BaseDevice.hpp"

#include "interface/ComInterface.hpp"

namespace eduart {

namespace sensorring {

namespace device {

BaseDevice::BaseDevice(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable)
    : _id(id)
    , _idx(id.getIndex())
    , _state(DeviceState::Undefined)
    , _enable(enable)
    , _interface(interface) {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
      },
      { target });
}

BaseDevice::~BaseDevice() {
  // _com_subscription auto-cancels via RAII.
}

void BaseDevice::enqueueAction(std::function<void()> action) {
  std::lock_guard<std::mutex> lock(_action_mutex);
  _pending_actions.push_back(std::move(action));
}

std::vector<std::function<void()> > BaseDevice::drainActions() {
  std::lock_guard<std::mutex> lock(_action_mutex);
  std::vector<std::function<void()> > actions;
  actions.swap(_pending_actions);
  return actions;
}

DeviceID BaseDevice::getDeviceID() const {
  return _id;
}

unsigned int BaseDevice::getIdx() const {
  return _idx;
}

void BaseDevice::setEnable(bool enable) {
  _enable = enable;
}

bool BaseDevice::getEnable() const {
  return _enable;
}

std::future<bool> BaseDevice::beginMeasurementWait() {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  _measurement_promise.reset();
  _measurement_promise.emplace();
  return _measurement_promise->get_future();
}

void BaseDevice::setMeasurementReady(bool success) {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  if (_measurement_promise) {
    try {
      _measurement_promise->set_value(success);
    } catch (const std::future_error&) {
    }
    _measurement_promise.reset();
  }
}

std::future<bool> BaseDevice::beginDataAvailableWait() {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  _data_available_promise.reset();
  _data_available_promise.emplace();
  return _data_available_promise->get_future();
}

void BaseDevice::setDataAvailableReady(bool success) {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  if (_data_available_promise) {
    try {
      _data_available_promise->set_value(success);
    } catch (const std::future_error&) {
    }
    _data_available_promise.reset();
  }
}

void BaseDevice::setPose(math::Vector3 translation, math::Vector3 rotation) {
  _translation = translation;
  _rotation    = rotation;
  _rot_m       = math::rotMatrixFromEulerDegrees(_rotation);
}

void BaseDevice::resetSensorState() {
  std::lock_guard<std::mutex> state_lock(_state_mutex);
  _state = DeviceState::Ok;

  {
    std::lock_guard<std::mutex> promise_lock(_promise_mutex);
    _data_available_promise.reset();
    _measurement_promise.reset();
  }

  onResetSensorState();
}

void BaseDevice::clearDataFlag() {
  std::lock_guard<std::mutex> state_lock(_state_mutex);
  _state = DeviceState::Ok;

  {
    std::lock_guard<std::mutex> promise_lock(_promise_mutex);
    _measurement_promise.reset();
  }

  onClearDataFlag();
}

} // namespace device

} // namespace sensorring

} // namespace eduart