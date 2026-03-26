#include "sensorring/device/BaseSensor.hpp"

#include "interface/ComInterface.hpp"
#include "sensorring/math/Math.hpp"

namespace eduart {

namespace device {

BaseSensor::BaseSensor(com::ComInterface* interface, com::ComEndpoint target, unsigned int idx, bool enable)
    : _idx(idx)
    , _error(SensorState::SensorInit)
    , _interface(interface)
    , _enable_flag(enable) {
  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, const std::vector<uint8_t>& data) {
        this->comCallback(source, data);
      },
      { target });
}

BaseSensor::~BaseSensor() {
  // _com_subscription auto-cancels via RAII.
}

unsigned int BaseSensor::getIdx() const {
  return _idx;
}

void BaseSensor::setEnable(bool enable) {
  _enable_flag = enable;
}

bool BaseSensor::getEnable() const {
  return _enable_flag;
}

std::future<bool> BaseSensor::beginMeasurementWait() {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  // Start a new measurement cycle by resetting any previous promise.
  _measurement_promise.reset();
  _measurement_promise.emplace();
  return _measurement_promise->get_future();
}

void BaseSensor::setMeasurementReady(bool success) {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  if (_measurement_promise) {
    try {
      _measurement_promise->set_value(success);
    } catch (const std::future_error&) {
      // Promise already satisfied or future gone; nothing more to do.
    }
    _measurement_promise.reset();
  }
}

std::future<bool> BaseSensor::beginDataAvailableWait() {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  _data_available_promise.reset();
  _data_available_promise.emplace();
  return _data_available_promise->get_future();
}

void BaseSensor::setDataAvailableReady(bool success) {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  if (_data_available_promise) {
    try {
      _data_available_promise->set_value(success);
    } catch (const std::future_error&) {
      // Promise already satisfied or future gone; nothing more to do.
    }
    _data_available_promise.reset();
  }
}

void BaseSensor::setPose(math::Vector3 translation, math::Vector3 rotation) {
  _translation = translation;
  _rotation    = rotation;
  _rot_m       = math::rotMatrixFromEulerDegrees(_rotation);
}

void BaseSensor::resetSensorState() {
  // Locking order: _state_mutex → _promise_mutex.
  std::lock_guard<std::mutex> state_lock(_state_mutex);
  _error = SensorState::SensorOK;

  {
    std::lock_guard<std::mutex> promise_lock(_promise_mutex);
    _data_available_promise.reset();
    _measurement_promise.reset();
  }

  onResetSensorState();
}

void BaseSensor::clearDataFlag() {
  // Locking order: _state_mutex → _promise_mutex.
  std::lock_guard<std::mutex> state_lock(_state_mutex);
  _error = SensorState::SensorOK;

  {
    std::lock_guard<std::mutex> promise_lock(_promise_mutex);
    _measurement_promise.reset();
  }

  onClearDataFlag();
}

} // namespace device

} // namespace eduart