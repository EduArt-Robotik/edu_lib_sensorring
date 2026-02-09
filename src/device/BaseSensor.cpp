#include "sensorring/device/BaseSensor.hpp"

#include "sensorring/math/Math.hpp"

namespace eduart {

namespace device {

BaseSensor::BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable)
    : ComObserver()
    , _idx(idx)
    , _error(SensorState::SensorInit)
    , _interface(interface)
    , _enable_flag(enable)
    , _new_data_available_flag(false)
    , _new_data_in_buffer_flag(false)
    , _new_measurement_ready_flag(false)

{
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

bool BaseSensor::gotNewData() const {
  return _new_measurement_ready_flag.load(std::memory_order_acquire);
}

bool BaseSensor::newDataAvailable() const {
  return _new_data_available_flag.load(std::memory_order_acquire);
}

void BaseSensor::setPose(math::Vector3 translation, math::Vector3 rotation) {
  _translation = translation;
  _rotation    = rotation;
  _rot_m       = math::rotMatrixFromEulerDegrees(_rotation);
}

void BaseSensor::resetSensorState() {
  std::lock_guard<std::mutex> lock(_state_mutex);
  _error = SensorState::SensorOK;
  _new_data_available_flag.store(false, std::memory_order_release);
  _new_data_in_buffer_flag.store(false, std::memory_order_release);
  _new_measurement_ready_flag.store(false, std::memory_order_release);
  onResetSensorState();
}

void BaseSensor::clearDataFlag() {
  std::lock_guard<std::mutex> lock(_state_mutex);
  _error = SensorState::SensorOK;
  _new_data_in_buffer_flag.store(false, std::memory_order_release);
  _new_measurement_ready_flag.store(false, std::memory_order_release);
  // Do not clear _new_data_available_flag here
  onClearDataFlag();
}

} // namespace device

} // namespace eduart