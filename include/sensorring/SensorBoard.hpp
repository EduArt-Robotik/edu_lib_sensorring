#pragma once

#include <memory>

#include "ComInterface.hpp"

#include "Parameters.hpp"
#include "TofSensor.hpp"
#include "ThermalSensor.hpp"
#include "LedLight.hpp"

namespace eduart{

namespace sensor{

enum class SensorBoardType{
    headlight,
    taillight,
    sidepanel,
    minipanel,
    unknown
};

class SensorBoard{
    public:
        SensorBoard(SensorBoardParams params, com::ComInterface* interface, std::size_t idx);
        ~SensorBoard();

        SensorBoardType getType() const;
        void setType(SensorBoardType type);

        //void tofSetEnable(bool enable);
        //void thermalSetEnable(bool enable);
        void tofClearDataFlag();
        void thermalClearDataFlag();
        void thermalReadEEPROM();
        bool thermalStopCalibration();
        bool thermalStartCalibration(size_t window);

        const TofSensor* getTof() const;
        const ThermalSensor* getThermal() const;
        const LedLight* getLed() const;
    
    private:
        SensorBoardType _sensor_type;

        std::unique_ptr<TofSensor>      _tof;
        std::unique_ptr<ThermalSensor>  _thermal;
        std::unique_ptr<LedLight>       _leds;
};

}

}