#include "sensors/BaseSensor.hpp"

#include "Math.hpp"

namespace eduart {

namespace sensor {

BaseSensor::BaseSensor(com::ComInterface* interface, com::ComEndpoint target, unsigned int idx, bool enable)
    : ComObserver()
    , _idx(idx)
    , _error(SensorState::SensorInit)
    , _interface(interface)
    , _enable_flag(enable)
    , _new_data_available_flag(false)
    , _new_data_in_buffer_flag(false)
    , _new_measurement_ready_flag(false)

{
  addEndpoint(target);
  _interface->registerObserver(this);
}

BaseSensor::~BaseSensor() {
  _interface->unregisterObserver(this);
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

bool BaseSensor::gotNewData() const {
  return _new_measurement_ready_flag;
}

bool BaseSensor::newDataAvailable() const {
  return _new_data_available_flag;
}

void BaseSensor::setPose(math::Vector3 translation, math::Vector3 rotation) {
  _translation = translation;
  _rotation    = rotation;
  _rot_m       = math::Matrix3::rotMatrixFromEulerDegrees(_rotation);
}

void BaseSensor::clearDataFlag() {
  _error                      = SensorState::SensorOK;
  _new_data_in_buffer_flag    = false;
  _new_measurement_ready_flag = false;
  onClearDataFlag();
}

void BaseSensor::notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  canCallback(source, data);
}

} // namespace sensor

} // namespace eduart