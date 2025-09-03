#include "sensors/BaseSensor.hpp"

namespace eduart{

namespace sensor{

BaseSensor::BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable) : ComObserver(),
    _idx(idx),
    _error(SensorState::SensorInit),
    _new_data_available_flag(false),
    _new_data_in_buffer_flag(false),
    _new_measurement_ready_flag(false),
    _enable_flag(enable),
    _target(target),
    _interface(interface)
{
    addEndpoint(target);    
    _interface->registerObserver(this);
}

BaseSensor::~BaseSensor(){
    _interface->unregisterObserver(this);
}

std::size_t BaseSensor::getIdx() const{
    return _idx;
}

/*void BaseSensor::enableCallback(){
    _can_interface->registerObserver(this);
};*/

bool BaseSensor::isEnabled() const {
    return _enable_flag;
}

bool BaseSensor::gotNewData() const {
    return _new_measurement_ready_flag;
}

bool BaseSensor::newDataAvailable() const {
    return _new_data_available_flag;
}

/*void BaseSensor::setEnable(bool enable){
    _enable_flag = enable;
};*/

void BaseSensor::clearDataFlag(){
    _error = SensorState::SensorOK;
    _new_data_in_buffer_flag = false;
    _new_measurement_ready_flag = false;
    onClearDataFlag();
}

void BaseSensor::notify(const com::ComEndpoint source, const std::vector<uint8_t>& data){
    canCallback(source, data);
}

}

}