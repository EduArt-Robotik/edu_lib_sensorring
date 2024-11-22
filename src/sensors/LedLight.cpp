#include "LedLight.hpp"

namespace sensor{

LedLight::LedLight(LightParams params){
    _params = params;
};

LedLight::~LedLight(){

};

const LightParams& LedLight::getParams() const{
    return _params;
};

}