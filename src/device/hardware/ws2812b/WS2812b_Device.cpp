#include "WS2812b_Device.hpp"

#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace device {

WS2812b_Device::WS2812b_Device(WS2812b_Params params, com::ComInterface* interface)
    : BaseDevice(DeviceID({ DeviceType::WS2812b, "light", 0 }), interface, com::ComEndpoint("light"), params.enable)
    , _params(params) {
  interface->addLightSensorEndpoint();
}

WS2812b_Device::~WS2812b_Device() {
}

const WS2812b_Params& WS2812b_Device::getParams() const {
  return _params;
}

bool WS2812b_Device::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  std::uint8_t mode_cmd       = static_cast<uint8_t>(mode) + CAN_LIGHT_LIGHTS_OFF;
  std::vector<uint8_t> tx_buf = { mode_cmd, red, green, blue };

  for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
    interface->send(com::ComEndpoint("light"), tx_buf);
  }
  return true;
}

bool WS2812b_Device::syncLight() {
  std::vector<uint8_t> tx_buf = { CAN_LIGHT_BEAT, 0x00 };

  for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
    interface->send(com::ComEndpoint("light"), tx_buf);
  }
  return true;
}

void WS2812b_Device::comCallback(const com::ComEndpoint, const std::vector<uint8_t>&) {
}

void WS2812b_Device::onResetSensorState() {
}

void WS2812b_Device::onClearDataFlag() {
}

} // namespace device

} // namespace eduart
