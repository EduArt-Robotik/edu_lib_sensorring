#include "WS2812b_Device.hpp"

#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"
#include "sensorring/device/IDeviceMacros.hpp"

namespace eduart {

namespace device {

SENSORRING_REGISTER_STATIC(WS2812b_Device, SetLight, &WS2812b_Device::setLight);
SENSORRING_REGISTER_STATIC(WS2812b_Device, SyncLight, &WS2812b_Device::syncLight);

WS2812b_Device::WS2812b_Device(WS2812b_Params params, com::ComInterface* interface)
    : BaseDevice(DeviceID({ DeviceType::WS2812b, "light", 0 }), interface, com::ComEndpoint("light"), params.enable)
    , _params(params) {

  SENSORRING_REGISTER_FUNCTION_NAMED(SetLight, &WS2812b_Device::setLight, "SetLight");
  SENSORRING_REGISTER_FUNCTION_NAMED(SyncLight, &WS2812b_Device::syncLight, "SyncLight");

  interface->addLightSensorEndpoint();
}

WS2812b_Device::~WS2812b_Device() {
}

const WS2812b_Params& WS2812b_Device::getParams() const {
  return _params;
}

SetLight::Response WS2812b_Device::setLight(const SetLight::Request& request) {
  std::uint8_t mode_cmd       = static_cast<uint8_t>(request.mode) + CAN_LIGHT_LIGHTS_OFF;
  std::vector<uint8_t> tx_buf = { mode_cmd, request.red, request.green, request.blue };

  for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
    interface->send(com::ComEndpoint("light"), tx_buf);
  }
  return SetLight::Response{ true };
}

SyncLight::Response WS2812b_Device::syncLight(const SyncLight::Request&) {
  std::vector<uint8_t> tx_buf = { CAN_LIGHT_BEAT, 0x00 };

  for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
    interface->send(com::ComEndpoint("light"), tx_buf);
  }
  return SyncLight::Response{ true };
}

void WS2812b_Device::comCallback(const com::ComEndpoint, const std::vector<uint8_t>&) {
}

void WS2812b_Device::onResetSensorState() {
}

void WS2812b_Device::onClearDataFlag() {
}

} // namespace device

} // namespace eduart
