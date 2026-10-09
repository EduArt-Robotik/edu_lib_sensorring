#include "sensorring/device/thermal/htpa32/HTPA32_Device.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "device/thermal/htpa32/HTPA32_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::htpa32;

namespace eduart {

namespace sensorring {

namespace device {

HTPA32_Device::HTPA32_Device(HTPA32_Params params, com::ComInterfaceID interface, unsigned int idx)
    : ThermalSensor(DeviceID({ DeviceType::HTPA32, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::HTPA32 })
    , _impl(std::make_unique<HTPA32_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
}

HTPA32_Device::~HTPA32_Device() {
}

const HTPA32_Params& HTPA32_Device::getParams() const {
  return _impl->getParams();
}

bool HTPA32_Device::configure() {
  return _impl->configure();
}

bool HTPA32_Device::stopCalibration() {
  return _impl->stopCalibration();
}

bool HTPA32_Device::startCalibration(unsigned int window) {
  return _impl->startCalibration(window);
}

void HTPA32_Device::onClearDataFlag() {
  _impl->onClearDataFlag();
}

void HTPA32_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  _impl->comCallback(source, command, data);
}

std::future<bool> HTPA32_Device::getEepromAsync(std::chrono::milliseconds timeout) {
  return _impl->getEepromAsync(timeout);
}

bool HTPA32_Device::sendMeasurementRequest(std::uint8_t) {
  return _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::HTPA32 }, MEASUREMENT_REQUEST, {});
}

bool HTPA32_Device::sendMeasurementTransmissionRequest() {
  return _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::HTPA32 }, MEASUREMENT_TRANSMISSION_REQUEST, {});
}

} // namespace device

} // namespace sensorring

} // namespace eduart
