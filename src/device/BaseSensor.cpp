#include "sensorring/device/BaseSensor.hpp"

#include "sensorring/math/Math.hpp"

namespace eduart {

namespace device {

BaseSensor::BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable)
    : ComObserver()
    , _idx(idx)
    , _error(SensorState::SensorInit)
    , _interface(interface)
    , _enable_flag(enable) {
  subscribeToEndpoint(target);
  _interface->registerObserver(this);
}

BaseSensor::~BaseSensor() {
  _interface->unregisterObserver(this);
}

std::size_t BaseSensor::getIdx() const {
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
  std::lock_guard<std::mutex> lock(_state_mutex);
  _error = SensorState::SensorOK;
  _data_available_promise.reset();
  _measurement_promise.reset();


  onResetSensorState();
}

void BaseSensor::clearDataFlag() {
  std::lock_guard<std::mutex> lock(_state_mutex);
  _error = SensorState::SensorOK;
  _measurement_promise.reset();

  onClearDataFlag();
}

} // namespace device

} // namespace eduart