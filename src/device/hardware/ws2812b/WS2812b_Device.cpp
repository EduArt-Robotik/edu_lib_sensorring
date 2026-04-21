#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "device/hardware/ws2812b/WS2812b_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::ws2812b;

namespace eduart {

namespace sensorring {

namespace device {

WS2812b_Device::WS2812b_Device(WS2812b_Params params, com::ComInterfaceID interface, unsigned int idx)
    : BaseDevice(DeviceID({ DeviceType::WS2812b, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::WS2812B }, params.enable)
    , _impl(std::make_unique<WS2812b_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface))) {
}

WS2812b_Device::~WS2812b_Device() {
}

const WS2812b_Params& WS2812b_Device::getParams() const {
  return _impl->getParams();
}

void WS2812b_Device::setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  enqueueAction([r, g, b]() {
    setLight(LightMode::FixedColor, r, g, b);
  });
}

void WS2812b_Device::setMode(LightMode mode) {
  enqueueAction([mode]() {
    setLight(mode, 0, 0, 0);
  });
}

bool WS2812b_Device::setLight(LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  std::uint8_t mode_cmd       = static_cast<uint8_t>(mode);
  std::vector<uint8_t> tx_buf = { mode_cmd, red, green, blue };

  for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
    interface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::WS2812B }, SET_LED_MODE, tx_buf);
  }
  return true;
}

bool WS2812b_Device::syncLight() {
  std::vector<uint8_t> tx_buf = {};

  for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
    interface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::WS2812B }, SYNCHRONIZE, tx_buf);
  }
  return true;
}

void WS2812b_Device::comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, command, data);
}

} // namespace device

} // namespace sensorring

} // namespace eduart
