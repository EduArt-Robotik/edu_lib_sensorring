#include "sensorring/SensorRingFactory.hpp"

#include "factory/SensorRingFactoryImpl.hpp"

namespace eduart {
namespace sensorring {

SensorRingFactory::SensorRingFactory(ValidationMode mode)
    : _impl(std::make_unique<SensorRingFactoryImpl>(mode)) {
}

SensorRingFactory::~SensorRingFactory() = default;

void SensorRingFactory::addInterface(com::SocketCanParams params) {
  _impl->addInterface(std::move(params));
}

void SensorRingFactory::addInterface(com::UsbTingoParams params) {
  _impl->addInterface(std::move(params));
}

void SensorRingFactory::expectBoard(board::SensorBoardParams params) {
  _impl->expectBoard(std::move(params));
}

void SensorRingFactory::expectDevice(device::VL53L8CX_Params params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::expectDevice(device::TMF8829_Params params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::expectDevice(device::HTPA32_Params params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::expectDevice(device::WS2812b_Params params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::expectDevice(device::DepthSensorParams params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::expectDevice(device::ThermalSensorParams params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::expectDevice(device::LightParams params) {
  _impl->expectDevice(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::VL53L8CX_Params params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::TMF8829_Params params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::HTPA32_Params params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::WS2812b_Params params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::DepthSensorParams params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::ThermalSensorParams params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::setDefaultDeviceParams(device::LightParams params) {
  _impl->setDefaultDeviceParams(std::move(params));
}

void SensorRingFactory::reset() {
  _impl->reset();
}

std::unique_ptr<SensorRing> SensorRingFactory::build() {
  return _impl->build();
}

SensorRingFactory::EnumerationMap SensorRingFactory::enumerate() {
  return _impl->enumerate();
}

const SensorRingFactory::EnumerationMap& SensorRingFactory::getLatestEnumerationResult() const {
  return _impl->getLatestEnumerationResult();
}

std::string SensorRingFactory::printTopology() const {
  return _impl->printTopology();
}

} // namespace sensorring
} // namespace eduart
