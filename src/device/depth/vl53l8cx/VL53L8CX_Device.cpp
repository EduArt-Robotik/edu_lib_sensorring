#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Device.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "device/depth/vl53l8cx/VL53L8CX_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::vl53l8cx;

namespace eduart {

namespace sensorring {

namespace device {

static constexpr DepthSensorConfig getVL53L8CXConfig() {
  DepthSensorConfig c;
  c.fov_x_deg                      = 45.0;
  c.fov_y_deg                      = 45.0;
  c.res_x                          = 8;
  c.res_y                          = 8;
  c.invert_x_lut                   = true;
  c.invert_y_lut                   = true;
  c.reports_perpendicular_distance = true;
  return c;
}

// clang-format off
VL53L8CX_Device::VL53L8CX_Device(VL53L8CX_Params params, com::ComInterfaceID interface, unsigned int idx)
  : DepthSensor( DeviceID({ DeviceType::VL53L8CX, idx}), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::VL53L8CX }, getVL53L8CXConfig()),
    _impl(std::make_unique<VL53L8CX_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
}
// clang-format on

VL53L8CX_Device::~VL53L8CX_Device() {
}

const VL53L8CX_Params& VL53L8CX_Device::getParams() const {
  return _impl->getParams();
}

void VL53L8CX_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, command, data);
}

bool VL53L8CX_Device::sendMeasurementRequest(std::uint8_t sequence_number) {
  return _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::VL53L8CX }, MEASUREMENT_REQUEST, { sequence_number });
}

bool VL53L8CX_Device::sendMeasurementTransmissionRequest() {
  return _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::VL53L8CX }, MEASUREMENT_TRANSMISSION_REQUEST, {});
}

} // namespace device

} // namespace sensorring

} // namespace eduart
