#include "sensors/BaseSensor.hpp"

namespace sensor{

BaseSensor::BaseSensor(std::shared_ptr<com::ComInterface> interface, com::Endpoint target, bool enable){
    _enable_flag = enable;

    _new_data_available_flag = false;
    _new_data_in_buffer_flag = false;
    _new_measurement_ready_flag = false;

    _target = target;
    addEndpoint(target);
    
    _interface = interface;
    _interface->registerObserver(this);
};

BaseSensor::~BaseSensor(){

};

/*void BaseSensor::enableCallback(){
    _can_interface->registerObserver(this);
};*/

bool BaseSensor::isEnabled() const {
    return _enable_flag;
};

bool BaseSensor::gotNewData() const {
    return _new_measurement_ready_flag;
};

bool BaseSensor::newDataAvailable() const {
    return _new_data_available_flag;
};

/*void BaseSensor::setEnable(bool enable){
    _enable_flag = enable;
};*/

void BaseSensor::clearDataFlag(){
    _error = SensorState::SensorOK;
    _new_data_in_buffer_flag = false;
    _new_measurement_ready_flag = false;
    onClearDataFlag();
};

void BaseSensor::notify(const com::Endpoint source, const std::vector<uint8_t>& data){
    canCallback(source, data);
};

};