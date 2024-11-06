#pragma once

#include "BaseSensor.hpp"

namespace Sensor{

struct LedLightParams{
    int can_id = 0;
    int nr_of_leds = 0;
    SensorOrientation orientation = SensorOrientation::none;
};

class LedLight{
    public:
        LedLight(LedLightParams params);
        ~LedLight();
    
        const LedLightParams& getParams() const;

    private:
        LedLightParams _params;

        int _canid_in;
        int _canid_out;
};

}; //namespace Sensor