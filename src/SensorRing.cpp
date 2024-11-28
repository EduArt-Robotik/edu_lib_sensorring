#include <cmath>

#include "SensorRing.hpp"
#include "canprotocol.hpp"
#include "Logger.hpp"



namespace ring{

SensorRing::SensorRing(RingParams params) : _params(params){

	if(_params.timeout == std::chrono::milliseconds(0)){
		Logger::getInstance()->log(LogVerbosity::Warning, "SensorRing timeout parameter is 0.0s");
	} else if(_params.timeout < std::chrono::milliseconds(200)){
		Logger::getInstance()->log(LogVerbosity::Error, "SensorRing timeout parameter of " + std::to_string(_params.timeout.count()) + " ms is probably too low");
	} 

	for(auto bus_params : params.bus_param_vec){
		_sensor_bus_vec.push_back(std::make_unique<bus::SensorBus>(bus_params));
	}
};

SensorRing::~SensorRing(){

};

std::vector<const bus::SensorBus*> SensorRing::getInterfaces() const{
	
	std::vector<const bus::SensorBus*> ref_vec;
    for(const auto& sensor_bus : _sensor_bus_vec){
        ref_vec.push_back(sensor_bus.get());
    }

    return ref_vec;
};

void SensorRing::resetDevices(){
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->resetDevices();
	}
};

bool SensorRing::enumerateDevices(){
	size_t sensor_count = 0;
	bool success = true;

	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_count = sensor_bus->enumerateDevices();
		success &= (sensor_bus->getSensorCount() == sensor_count);
	}

	return success;
};

void SensorRing::syncLight(){
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->syncLights();
	}

	unsigned char red = 0, green = 0, blue = 0;
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->setLights(CAN_LIGHT_PULSATION, red, green, blue);
	}
};

bool SensorRing::getEEPROM(){

	// request transmission of eeprom from all thermal sensors
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->requestEEPROM();
	}

	// wait until all sensors sent their response. 1 sec timeout
    bool ready = false;
	auto timestamp = std::chrono::steady_clock::now();

    while(!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout){
		ready = true;
		for(auto& sensor_bus : _sensor_bus_vec){
			ready &= sensor_bus->allEEPROMTransmissionsComplete();
		}
		
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}

	return ready;
};

void SensorRing::requestTofMeasurement(){
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->requestTofMeasurement();
	}
};

bool SensorRing::waitForAllTofMeasurementsReady() const{
	
	bool ready = false;
	auto timestamp = std::chrono::steady_clock::now();
	
	while(!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout){
		ready = true;
		for(auto& sensor_bus : _sensor_bus_vec){
			ready &= sensor_bus->allTofMeasurementsReady();
		}

		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	
	return ready;
};

bool SensorRing::waitForAllThermalMeasurementsReady() const{
	// int watchdog = 0;
	// bool ready = false;

	// while(!ready && watchdog < _params.timeout){
	// 	ready = true;
	// 	for(auto& sensor_bus : _sensor_bus_vec){
	// 		ready &= sensor_bus->allThermalMeasurementsReady();
	// 	}

	// 	watchdog += 100;
	// 	std::this_thread::sleep_for(std::chrono::microseconds(100);
	// }
	
	// return ready; // ToDo: Check if a thermal frame is actually available
	return true;
}

bool SensorRing::waitForAllTofDataTransmissionsComplete() const{
	bool ready = false;
	auto timestamp = std::chrono::steady_clock::now();

	while(!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout){
		ready = true;
		for(auto& sensor_bus : _sensor_bus_vec){
			ready &= sensor_bus->allTofDataTransmissionsComplete();
		}

		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	
	return ready;
};

bool SensorRing::waitForAllThermalDataTransmissionsComplete() const{
	bool ready = false;
	auto timestamp = std::chrono::steady_clock::now();

	while(!ready && (std::chrono::steady_clock::now() - timestamp) < _params.timeout){
		ready = true;
		for(auto& sensor_bus : _sensor_bus_vec){
			ready &= sensor_bus->allThermalDataTransmissionsComplete();
		}

		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	
	return ready;
};

void SensorRing::fetchTofData(){
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->fetchTofData();
	}
};

void SensorRing::requestThermalMeasurement(){
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->requestThermalMeasurement();
	}
};

void SensorRing::fetchThermalData(){  
	for(auto& sensor_bus : _sensor_bus_vec){
		sensor_bus->fetchThermalData();
	}
};

bool SensorRing::stopThermalCalibration(){
    bool success = true;

    for (auto& sensor_bus : _sensor_bus_vec){
        success &= sensor_bus->stopThermalCalibration();
    }

    return success;
};

bool SensorRing::startThermalCalibration(size_t window){
    bool success = true;

    for (auto& sensor_bus : _sensor_bus_vec){
        success &= sensor_bus->startThermaltCalibration(window);
    }

    return success;
};

} //namespace SensorRing