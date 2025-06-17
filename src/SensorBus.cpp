#include "SensorBus.hpp"

#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"

#include <string>
#include <chrono>

namespace eduart{

namespace bus{

SensorBus::SensorBus(BusParams params) : _params(params){

    auto interface = com::ComManager::getInstance()->createInterface(params.type, params.interface_name, params.board_param_vec.size());
    //auto interface = com::ComManager::getInstance()->getInterface(params.interface_name);

    std::size_t idx = 0;
    for(auto board_params : params.board_param_vec){    
        _sensor_board_vec.push_back(std::make_unique<sensor::SensorBoard>(board_params, interface, idx));
        idx++;
    }

    _enumerate_flag = false;
    _enumerate_count = 0;

    _active_tof_sensors = 0;
    _active_thermal_sensors = 0;
    _tof_measurement_count = 0;
    _thermal_measurement_count = 0;

    // add my own callback
    addEndpoint(com::ComEndpoint("broadcast"));
    addEndpoint(com::ComEndpoint("tof_status"));
    addEndpoint(com::ComEndpoint("thermal_status"));
    _interface = com::ComManager::getInstance()->getInterface(_params.interface_name);
    _interface->registerObserver(this);
}

SensorBus::~SensorBus(){

}

const std::string SensorBus::getInterfaceName() const{
    return _params.interface_name;
}

std::vector<const sensor::SensorBoard*> SensorBus::getSensorBoards() const{

    std::vector<const sensor::SensorBoard*> ref_vec;
    for(const auto& sensor : _sensor_board_vec){
        ref_vec.push_back(sensor.get());
    }

    return ref_vec;
}

bool SensorBus::isTofEnabled(int idx) const{
    if(idx >= 0 && idx < (int)_sensor_board_vec.size()){
        return _sensor_board_vec[idx]->getTof()->isEnabled();
    }
    return false;
}

bool SensorBus::isThermalEnabled(int idx) const{
    if(idx >= 0 && idx < (int)_sensor_board_vec.size()){
        return _sensor_board_vec[idx]->getThermal()->isEnabled();
    }
    return false;
}

size_t SensorBus::getSensorCount() const{
    return _sensor_board_vec.size();
}

size_t SensorBus::getEnumerationCount() const{
    return _enumerate_count;
}

void SensorBus::syncLight(){

    std::vector<uint8_t> tx_buf = {CAN_LIGHT_BEAT, 0x00};
    _interface->send(com::ComEndpoint("light"), tx_buf);
}

void SensorBus::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue){

    // Map the enum class value to the corresponding index in the command
    std::uint8_t mode_idx = static_cast<uint8_t>(mode) + 2;
    
    std::vector<uint8_t> tx_buf = {mode_idx, red, green, blue};
    _interface->send(com::ComEndpoint("light"), tx_buf);
}

void SensorBus::resetDevices(){
    std::vector<uint8_t> tx_buf = {CMD_HARD_RESET};
    _interface->send(com::ComEndpoint("broadcast"), tx_buf);
}

int SensorBus::enumerateDevices(){
    _enumerate_flag = true;
    _enumerate_count = 0;

    std::vector<uint8_t> tx_buf = {CMD_ACTIVE_DEVICE_QUERY, CMD_ACTIVE_DEVICE_QUERY};
    _interface->send(com::ComEndpoint("broadcast"), tx_buf);

    // wait until all sensors sent their response. 100 ms timeout
    unsigned int watchdog = 0;
    while(_enumerate_count < getSensorCount() && watchdog < 1e3){
        watchdog += 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // wait a little longer in case there are more sensors than specified
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    _enumerate_flag = false;
    return _enumerate_count;
}

void SensorBus::requestEEPROM(){
    int active_devices = 0;
    for(auto& sensor : _sensor_board_vec){
        // enable eeprom reading in thermal sensors
        sensor->thermalReadEEPROM();
        // check which boards have an active thermal sensor
        active_devices |= (sensor->getThermal()->isEnabled() && !sensor->getThermal()->gotEEPROM()) << sensor->getThermal()->getIdx();
    }
    
    if(active_devices != 0){
        uint8_t sensor_select_high = (uint8_t) ((active_devices >> 8) & 0xFF);
        uint8_t sensor_select_low  = (uint8_t) ((active_devices >> 0) & 0xFF);
        std::vector<uint8_t> tx_buf = {CMD_THERMAL_EEPROM_REQUEST, sensor_select_high, sensor_select_low};
        _interface->send(com::ComEndpoint("thermal_request"), tx_buf);
    }
}

bool SensorBus::allEEPROMTransmissionsComplete() const{
    bool ready = true;

    for(auto& sensor : _sensor_board_vec){
        if(sensor->getThermal()->isEnabled()){
            ready &= sensor->getThermal()->gotEEPROM();
        }
    }
    
    return ready;
}

void SensorBus::requestTofMeasurement(){
    int active_devices = 0;
    _active_tof_sensors = 0;
    _tof_measurement_count = 0;

    for(auto& sensor : _sensor_board_vec){
        // check which boards have an active tof sensor
        if(sensor->getTof()->isEnabled()){
            active_devices |= 1 << sensor->getTof()->getIdx();
            _active_tof_sensors ++;
        }
    }
    
    uint8_t sensor_select_high = (uint8_t) ((active_devices >> 8) & 0xFF);
    uint8_t sensor_select_low  = (uint8_t) ((active_devices >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = {CMD_TOF_SCAN_REQUEST, sensor_select_high, sensor_select_low};
    _interface->send(com::ComEndpoint("tof_request"), tx_buf);
}

void SensorBus::fetchTofData(){
    int active_devices = 0;
    for(auto& sensor : _sensor_board_vec){
        sensor->tofClearDataFlag();
        // check which boards have an active tof sensor
        active_devices |= sensor->getTof()->isEnabled() << sensor->getTof()->getIdx();
    }
    
    uint8_t sensor_select_high = (uint8_t) ((active_devices >> 8) & 0xFF);
    uint8_t sensor_select_low  = (uint8_t) ((active_devices >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = {sensor_select_high, sensor_select_low};
    _interface->send(com::ComEndpoint("tof_request"), tx_buf);
}

void SensorBus::requestThermalMeasurement(){
    int active_devices = 0;
    _active_thermal_sensors = 0;
    _thermal_measurement_count = 0;

    for(auto& sensor : _sensor_board_vec){
        // check which boards have an active thermal sensor
        if(sensor->getThermal()->isEnabled()){
            active_devices |= 1 << sensor->getThermal()->getIdx();
            _active_thermal_sensors ++;
        }
    }
    
    uint8_t sensor_select_high = (uint8_t) ((active_devices >> 8) & 0xFF);
    uint8_t sensor_select_low  = (uint8_t) ((active_devices >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = {CMD_THERMAL_SCAN_REQUEST, sensor_select_high, sensor_select_low};
    _interface->send(com::ComEndpoint("thermal_request"), tx_buf);
}

void SensorBus::fetchThermalData(){
    int active_devices = 0;
    for(auto& sensor : _sensor_board_vec){
        sensor->thermalClearDataFlag();
        // check which boards have an active tof sensor
        active_devices |= sensor->getThermal()->isEnabled() << sensor->getThermal()->getIdx();
    }
    
    uint8_t sensor_select_high = (uint8_t) ((active_devices >> 8) & 0xFF);
    uint8_t sensor_select_low  = (uint8_t) ((active_devices >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = {CMD_THERMAL_DATA_REQUEST, sensor_select_high, sensor_select_low};
    _interface->send(com::ComEndpoint("thermal_request"), tx_buf);
}

bool SensorBus::allTofMeasurementsReady() const{
    bool ready = true;
    for(auto& sensor : _sensor_board_vec){
        ready &= sensor->getTof()->newDataAvailable();
    }
    return ready;
}

bool SensorBus::allTofMeasurementsReady(int &ready_sensors_count) const{
    bool ready = true;
    ready_sensors_count = 0;
    for(auto& sensor : _sensor_board_vec){
        ready &= sensor->getTof()->newDataAvailable();
        if(ready) ready_sensors_count ++;
    }
    return ready;
}

bool SensorBus::allThermalMeasurementsReady() const{
    return _active_thermal_sensors == _thermal_measurement_count;
}

bool SensorBus::allThermalMeasurementsReady(int &ready_sensors_count) const{
    ready_sensors_count = _thermal_measurement_count;
    return _active_thermal_sensors == _thermal_measurement_count;
}

bool SensorBus::allTofDataTransmissionsComplete() const{
    unsigned int tof_data_count = 0;
    for (auto& sensor : _sensor_board_vec){
        tof_data_count += sensor->getTof()->gotNewData();
    }

    return _active_tof_sensors == tof_data_count;
}

bool SensorBus::allTofDataTransmissionsComplete(int &ready_sensors_count) const{
    unsigned int tof_data_count = 0;
    for (auto& sensor : _sensor_board_vec){
        tof_data_count += sensor->getTof()->gotNewData();
    }

    ready_sensors_count = tof_data_count;
    return _active_tof_sensors == tof_data_count;
}

bool SensorBus::allThermalDataTransmissionsComplete() const{
    unsigned int thermal_data_count = 0;
    for (auto& sensor : _sensor_board_vec){
        thermal_data_count += sensor->getThermal()->gotNewData();
    }

    return _active_thermal_sensors == thermal_data_count;
}

bool SensorBus::allThermalDataTransmissionsComplete(int &ready_sensors_count) const{
    unsigned int thermal_data_count = 0;
    for (auto& sensor : _sensor_board_vec){
        thermal_data_count += sensor->getThermal()->gotNewData();
    }

    ready_sensors_count = thermal_data_count;
    return _active_thermal_sensors == thermal_data_count;
}

bool SensorBus::stopThermalCalibration(){
    bool success = true;

    for (auto& sensor : _sensor_board_vec){
        success &= sensor->thermalStopCalibration();
    }

    return success;
}

bool SensorBus::startThermalCalibration(size_t window){
    bool success = true;

    for (auto& sensor : _sensor_board_vec){
        success &= sensor->thermalStartCalibration(window);
    }

    return success;
}

void SensorBus::notify(const com::ComEndpoint source, const std::vector<uint8_t>& data){
    // general sensor board status
    if(source == com::ComEndpoint("broadcast")){
        // enumeration message
        if(data[0] == CMD_ACTIVE_DEVICE_RESPONSE && data.size() == 3 && _enumerate_flag){
            _enumerate_count++;

            size_t idx = data[1] - 1;
            if(idx < getSensorCount()){
                if(data[2] == 0) _sensor_board_vec[idx]->setType(sensor::SensorBoardType::sidepanel);
                if(data[2] == 1) _sensor_board_vec[idx]->setType(sensor::SensorBoardType::headlight);
                if(data[2] == 2) _sensor_board_vec[idx]->setType(sensor::SensorBoardType::taillight);
                if(data[2] == 3) _sensor_board_vec[idx]->setType(sensor::SensorBoardType::minipanel);
            }
        }
        
    // tof sensor status
    }else if(source == com::ComEndpoint("tof_status")){
        // tof measurement finished
        //if(data.size() == 1){
        //    _tof_measurement_count ++;
        //}
    

    // thermal sensor status
    }else if(source == com::ComEndpoint("thermal_status")){
    
    }
}

}

}