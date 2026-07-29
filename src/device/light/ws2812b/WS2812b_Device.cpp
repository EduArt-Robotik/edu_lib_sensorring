#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::ws2812b;

namespace eduart {

namespace sensorring {

namespace device {

WS2812b_Device::WS2812b_Device(WS2812b_Params params, com::ComInterfaceID interface, unsigned int idx)
    : Light(DeviceID({ DeviceType::WS2812b, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::WS2812B })
    , _params(std::move(params))
    , _last_setting({}) {
}

bool WS2812b_Device::configure() {
  setLight(_last_setting.mode, _last_setting.red, _last_setting.green, _last_setting.blue);
  return true;
}

void WS2812b_Device::setLight(LightMode mode, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  _last_setting = { mode, r, g, b };

  auto* iface     = _interface;
  auto board_addr = static_cast<std::uint8_t>(_hw_idx + 1);
  execute("setLight", [mode, r, g, b, iface, board_addr]() {
    std::uint8_t mode_cmd       = static_cast<uint8_t>(mode);
    std::vector<uint8_t> tx_buf = { mode_cmd, r, g, b };
    iface->send(com::ComEndpoint{ com::Direction::Input, board_addr, devbyte::WS2812B }, SET_LED_MODE, tx_buf);
  });
}

void WS2812b_Device::syncLight() {
  globalExecute([]() {
    std::vector<uint8_t> tx_buf = {};
    for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
      interface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::WS2812B }, SYNCHRONIZE, tx_buf);
    }
  });
}

void WS2812b_Device::comCallback(const com::ComEndpoint, std::uint8_t, const std::vector<uint8_t>&) {
}

} // namespace device

} // namespace sensorring

} // namespace eduart
