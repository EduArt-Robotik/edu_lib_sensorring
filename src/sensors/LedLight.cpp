#include "LedLight.hpp"

namespace sensor{

LedLight::LedLight(LightParams params){
    _params = params;
    // ToDo: assign individual parameters to each light
    //make_can_ids(SYSID_LIGHT, NODEID_HEADLEFT);
    //make_can_ids(SYSID_LIGHT, NODEID_HEADRIGHT);
    //make_can_ids(SYSID_LIGHT, NODEID_TAILLEFT);
    //make_can_ids(SYSID_LIGHT, NODEID_TAILLEFT);
};

LedLight::~LedLight(){

};

const LightParams& LedLight::getParams() const{
    return _params;
};

}