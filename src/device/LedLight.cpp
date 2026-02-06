#include "LedLight.hpp"

#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace device {

LedLight::LedLight(LightParams params, com::ComInterface* interface)
    : _params(params) {

  register_function<SetLight>(&LedLight::setLight, "SetLight");
  register_function<SyncLight>(&LedLight::syncLight, "SyncLight");

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