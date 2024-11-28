#include "SensorBoard.hpp"

namespace sensor{

SensorBoard::SensorBoard(SensorBoardParams params, com::ComInterface* interface){
	_tof        = std::make_unique<TofSensor>(params.tof_params, interface, params.idx, params.enable_tof);
	_thermal    = std::make_unique<ThermalSensor>(params.thermal_params, interface, params.idx, params.enable_thermal);
	_leds       = std::make_unique<LedLight>(params.led_params);

	_sensor_type = SensorBoardType::unknown;
}

SensorBoard::~SensorBoard(){

}

SensorBoardType SensorBoard::getType() const{
	return _sensor_type;
}

void SensorBoard::setType(SensorBoardType type){
	if(_sensor_type == SensorBoardType::unknown){
		_sensor_type = type;
	}
}

void SensorBoard::tofClearDataFlag(){
	_tof->clearDataFlag();
}

void SensorBoard::thermalClearDataFlag(){
	_thermal->clearDataFlag();
}

void SensorBoard::thermalReadEEPROM(){
	_thermal->readEEPROM();
}

bool SensorBoard::thermalStopCalibration(){
	return _thermal->stopCalibration();
}

bool SensorBoard::thermalStartCalibration(size_t window){
	return _thermal->startCalibration(window);
}

const TofSensor* SensorBoard::getTof() const{
	return _tof.get();
}

const ThermalSensor* SensorBoard::getThermal() const{
	return _thermal.get();
}

const LedLight* SensorBoard::getLed() const{
	return _leds.get();
}

}