#pragma once

#include "BaseSensor.hpp"
#include "Parameters.hpp"

namespace sensor{

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