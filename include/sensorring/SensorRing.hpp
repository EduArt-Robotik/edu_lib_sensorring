#pragma once

#include <vector>
#include <memory>

#include "SensorBus.hpp"
#include "Parameters.hpp"

namespace eduart{

namespace ring{

class SensorRing{

public:
    SensorRing(RingParams params);
    ~SensorRing();
    
    std::vector<const bus::SensorBus*> getInterfaces() const;

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

    const RingParams _params;
    std::vector<std::unique_ptr<bus::SensorBus>> _sensor_bus_vec;
};

}

}