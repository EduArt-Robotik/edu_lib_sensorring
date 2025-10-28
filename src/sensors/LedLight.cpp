#include "LedLight.hpp"

#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace sensor {

LedLight::LedLight(LightParams params)
    : _params(params) {
}

LedLight::~LedLight() {
}

const LightParams& LedLight::getParams() const {
  return _params;
}

void LedLight::cmdSetLight(com::ComInterface* interface, light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {

  std::uint8_t mode_cmd       = static_cast<uint8_t>(mode) + CAN_LIGHT_LIGHTS_OFF;
  std::vector<uint8_t> tx_buf = { mode_cmd, red, green, blue };
  interface->send(com::ComEndpoint("light"), tx_buf);
}

void LedLight::cmdSyncLight(com::ComInterface* interface) {
  std::vector<uint8_t> tx_buf = { CAN_LIGHT_BEAT, 0x00 };
  interface->send(com::ComEndpoint("light"), tx_buf);
}

} // namespace sensor
} // namespace eduart