#pragma once

//#include <string>
#include <vector>
#include <memory>

#include "SensorBus.hpp"
#include "Parameters.hpp"

namespace sensorring{

class SensorRing{

public:
    SensorRing(SensorRingParams params);
    ~SensorRing();
    
    std::vector<const sensorbus::SensorBus*> getInterfaces() const;

    void syncLight();
    void resetDevices();
    bool enumerateDevices();
    bool getEEPROM();
    void requestTofMeasurement();
    void fetchTofData();
    void requestThermalMeasurement();
    void fetchThermalData();
    bool stopThermalCalibration();
    bool startThermalCalibration(size_t window);

    bool waitForAllTofMeasurementsReady() const;
    bool waitForAllTofDataTransmissionsComplete() const;
    bool waitForAllThermalMeasurementsReady() const;
    bool waitForAllThermalDataTransmissionsComplete() const;

private:

    std::vector<std::unique_ptr<sensorbus::SensorBus>> _sensor_bus_vec;
    //SensorBus::SensorBus _testbus;
};

} // namespace SensorRing