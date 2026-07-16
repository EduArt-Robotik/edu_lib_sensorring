#include "sensorring/manager/MeasurementManager.hpp"

#include "manager/MeasurementManagerImpl.hpp"

namespace eduart {

namespace sensorring {

namespace manager {

MeasurementManager::MeasurementManager(ManagerParams params, SensorRingFactory& factory)
    : _mm_impl(std::make_unique<MeasurementManagerImpl>(params, factory.build())) {
}

MeasurementManager::MeasurementManager(ManagerParams params, std::unique_ptr<SensorRing> sensor_ring)
    : _mm_impl(std::make_unique<MeasurementManagerImpl>(params, std::move(sensor_ring))) {
}

MeasurementManager::~MeasurementManager() noexcept {
}

ManagerParams MeasurementManager::getParams() const noexcept {
  return _mm_impl->getParams();
}

ManagerState MeasurementManager::getManagerState() const noexcept {
  return _mm_impl->getManagerState();
}

subscription::Subscription MeasurementManager::subscribeToStateChanges(std::function<void(ManagerState state)> callback) {
  return _mm_impl->subscribeToStateChanges(std::move(callback));
}

bool MeasurementManager::measureSome() noexcept {
  return _mm_impl->measureSome();
}

bool MeasurementManager::startMeasuring() noexcept {
  return _mm_impl->startMeasuring();
}

bool MeasurementManager::stopMeasuring() noexcept {
  return _mm_impl->stopMeasuring();
}

bool MeasurementManager::isMeasuring() noexcept {
  return _mm_impl->isMeasuring();
}

device::Group<device::DepthSensor> MeasurementManager::depthSensors() const noexcept {
  return _mm_impl->depthSensors();
}

device::Group<device::ThermalSensor> MeasurementManager::thermalSensors() const noexcept {
  return _mm_impl->thermalSensors();
}

device::Group<device::Light> MeasurementManager::lights() const noexcept {
  return _mm_impl->lights();
}

SensorRing* MeasurementManager::getRing() const noexcept {
  return _mm_impl->getRing();
}

} // namespace manager

} // namespace sensorring

} // namespace eduart