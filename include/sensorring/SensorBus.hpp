#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ComObserver.hpp"
#include "SensorBoard.hpp"
#include "Parameters.hpp"

namespace bus{


class SensorBus : public com::ComObserver{
    public:
        SensorBus(BusParams params);
        ~SensorBus();

        const std::string getInterfaceName() const;
        std::vector<const sensor::SensorBoard*> getSensorBoards() const;
        bool isTofEnabled(int idx) const;
        bool isThermalEnabled(int idx) const;
        size_t getSensorCount() const;
        size_t getEnumerationCount() const;

        bool allTofMeasurementsReady() const;
        bool allTofMeasurementsReady(int &ready_sensors_count) const;
        bool allThermalMeasurementsReady() const;
        bool allThermalMeasurementsReady(int &ready_sensors_count) const;
        bool allTofDataTransmissionsComplete() const;
        bool allTofDataTransmissionsComplete(int &ready_sensors_count) const;
        bool allThermalDataTransmissionsComplete() const;
        bool allThermalDataTransmissionsComplete(int &ready_sensors_count) const;
        bool allEEPROMTransmissionsComplete() const;

        void resetDevices();
        int enumerateDevices();
        void syncLights();
        void setLights(int mode, unsigned char red, unsigned char green, unsigned char blue);
        void requestEEPROM();
        void requestTofMeasurement();
        void fetchTofData();
        void requestThermalMeasurement();
        void fetchThermalData();

        bool stopThermalCalibration();
		bool startThermaltCalibration(size_t window);
        
        void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data);

    private:
        BusParams _params;
        std::vector<std::unique_ptr<sensor::SensorBoard>> _sensor_board_vec;
        
        volatile bool _enumerate_flag;
        volatile size_t _enumerate_count;

        size_t _active_tof_sensors;
        size_t _active_thermal_sensors;
        size_t _tof_measurement_count;
        size_t _thermal_measurement_count;

        com::ComInterface* _can_interface;
};

};