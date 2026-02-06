#include "LedLight.hpp"

#include "interface/can/canprotocol.hpp"
#include "sensorring/device/IDeviceMacros.hpp"

namespace eduart {

namespace device {

SENSORRING_REGISTER_STATIC(LedLight, SetLight, &LedLight::setLight);
SENSORRING_REGISTER_STATIC(LedLight, SyncLight, &LedLight::syncLight);

LedLight::LedLight(LightParams params, com::ComInterface* interface)
    : _params(params) {

  SENSORRING_REGISTER_FUNCTION_NAMED(SetLight, &LedLight::setLight, "SetLight");
  SENSORRING_REGISTER_FUNCTION_NAMED(SyncLight, &LedLight::syncLight, "SyncLight");

  interface->addLightSensorEndpoint();
}

LedLight::~LedLight() {
}

const LightParams& LedLight::getParams() const {
  return _params;
}

SetLight::Response LedLight::setLight(const SetLight::Request& request) {
  std::uint8_t mode_cmd       = static_cast<uint8_t>(request.mode) + CAN_LIGHT_LIGHTS_OFF;
  std::vector<uint8_t> tx_buf = { mode_cmd, request.red, request.green, request.blue };
  request.interface->send(com::ComEndpoint("light"), tx_buf);
  return SetLight::Response{ true };
}

SyncLight::Response LedLight::syncLight(const SyncLight::Request& request) {
  std::vector<uint8_t> tx_buf = { CAN_LIGHT_BEAT, 0x00 };
  request.interface->send(com::ComEndpoint("light"), tx_buf);
  return SyncLight::Response{ true };
}

} // namespace device

} // namespace eduart