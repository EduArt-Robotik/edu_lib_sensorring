#include "SensorBoard.hpp"

namespace eduart{

namespace sensor{

SensorBoard::SensorBoard(SensorBoardParams params, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds) :
	_params{params},
	_sensor_type(SensorBoardType::unknown)
{
	_tof        = std::move(tof);
	_thermal    = std::move(thermal);
	_leds       = std::move(leds);

}

SensorBoard::~SensorBoard(){

}

SensorBoardType SensorBoard::getType() const{
	return _sensor_type;
}

void SensorBoard::setType(SensorBoardType type){
	_sensor_type = type;
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

}