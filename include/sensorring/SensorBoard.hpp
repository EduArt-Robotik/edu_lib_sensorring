#pragma once

#include <memory>

#include "can/SocketCANFD.hpp"

#include "sensors/TofSensor.hpp"
#include "sensors/ThermalSensor.hpp"
#include "sensors/LedLight.hpp"

namespace Sensor{

enum class SensorBoardType{
    headlight,
    taillight,
    sidepanel,
    minipanel,
    unknown
};

struct SensorBoardParams{
    std::shared_ptr<edu::SocketCANFD> can_interface;
    canid_t canid_tof;
    canid_t canid_thermal;

    bool enable_tof;
    bool enable_thermal;

    TofSensorParams tof_params;
    ThermalSensorParams thermal_params;
    LedLightParams led_params;
};

class SensorBoard /*: public edu::SocketCANFDObserver*/{
    public:
        SensorBoard(SensorBoardParams params);
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

        //void notify(struct canfd_frame* frame) override;
    
    private:
        SensorBoardType _sensor_type;

        std::unique_ptr<TofSensor>      _tof;
        std::unique_ptr<ThermalSensor>  _thermal;
        std::unique_ptr<LedLight>       _leds;
};

} // namespace Sensor