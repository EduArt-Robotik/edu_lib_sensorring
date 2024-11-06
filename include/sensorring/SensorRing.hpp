#pragma once

//#include <string>
#include <vector>
#include <memory>

#include "SensorBus.hpp"

namespace SensorRing{

struct SensorRingParams{
    std::vector<SensorBus::SensorBusParams> bus_param_vec;
};

class SensorRing{

public:
    SensorRing(SensorRingParams params);
    ~SensorRing();
    
    std::vector<const SensorBus::SensorBus*> getInterfaces() const;

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

    std::vector<std::unique_ptr<SensorBus::SensorBus>> _sensor_bus_vec;
    //SensorBus::SensorBus _testbus;
};

} // namespace SensorRing