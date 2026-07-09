#include "sensorring/device/Sensor.hpp"

namespace eduart {

namespace sensorring {

namespace device {

Sensor::Sensor(DeviceID id, com::ComInterface* interface, com::ComEndpoint target)
    : Device(id, interface, target)
    , _state(DeviceState::Undefined) {
}

Sensor::~Sensor() = default;

std::future<bool> Sensor::beginMeasurementWait() {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  _measurement_promise.reset();
  _measurement_promise.emplace();
  return _measurement_promise->get_future();
}

void Sensor::setMeasurementReady(bool success) {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  if (_measurement_promise) {
    try {
      _measurement_promise->set_value(success);
    } catch (const std::future_error&) {
    }
    _measurement_promise.reset();
  }
}

std::future<bool> Sensor::beginDataAvailableWait() {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  _data_available_promise.reset();
  _data_available_promise.emplace();
  return _data_available_promise->get_future();
}

void Sensor::setDataAvailableReady(bool success) {
  std::lock_guard<std::mutex> lock(_promise_mutex);

  if (_data_available_promise) {
    try {
      _data_available_promise->set_value(success);
    } catch (const std::future_error&) {
    }
    _data_available_promise.reset();
  }
}

void Sensor::resetSensorState() {
  std::lock_guard<std::mutex> state_lock(_state_mutex);
  _state = DeviceState::Ok;

  {
    std::lock_guard<std::mutex> promise_lock(_promise_mutex);
    _data_available_promise.reset();
    _measurement_promise.reset();
  }

  onResetSensorState();
}

void Sensor::clearDataFlag() {
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
