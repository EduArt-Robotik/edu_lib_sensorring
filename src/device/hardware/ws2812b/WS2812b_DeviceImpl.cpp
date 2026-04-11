#include "device/hardware/ws2812b/WS2812b_DeviceImpl.hpp"

#include "interface/ComInterface.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"

namespace eduart {

namespace device {

WS2812b_DeviceImpl::WS2812b_DeviceImpl(WS2812b_Device& parent, WS2812b_Params params, com::ComInterface* interface)
    : _parent(parent)
    , _params(params) {
  (void)interface;
}

WS2812b_DeviceImpl::~WS2812b_DeviceImpl() {
}

const WS2812b_Params& WS2812b_DeviceImpl::getParams() const {
  return _params;
}

void WS2812b_DeviceImpl::comCallback(const com::ComEndpoint, std::uint8_t, const std::vector<uint8_t>&) {
  // WS2812b currently does not receive data; this is a no-op.
}

void WS2812b_DeviceImpl::onResetSensorState() {
  // No sensor state to reset for the WS2812b light.
}

void WS2812b_DeviceImpl::onClearDataFlag() {
  // No buffered data to clear for the WS2812b light.
}

} // namespace device

} // namespace eduart
