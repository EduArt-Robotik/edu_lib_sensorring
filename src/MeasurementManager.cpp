#include "sensorring/MeasurementManager.hpp"

#include <memory>

#include "MeasurementManagerImpl.hpp"

namespace eduart {

namespace manager {

MeasurementManager::MeasurementManager(ManagerParams params)
    : _mm_impl(std::make_unique<MeasurementManagerImpl>(params)) {
}

MeasurementManager::~MeasurementManager() noexcept {
}

void MeasurementManager::enableTofMeasurement(bool state) noexcept {
  return _mm_impl->enableTofMeasurement(state);
}

void MeasurementManager::enableThermalMeasurement(bool state) noexcept {
  return _mm_impl->enableThermalMeasurement(state);
}

ManagerParams MeasurementManager::getParams() const noexcept {
  return _mm_impl->getParams();
}

std::string MeasurementManager::printTopology() const noexcept {
  return _mm_impl->printTopology();
}

bool MeasurementManager::stopThermalCalibration() noexcept {
  return _mm_impl->stopThermalCalibration();
}

bool MeasurementManager::startThermalCalibration(std::size_t window) noexcept {
  return _mm_impl->startThermalCalibration(window);
}

void MeasurementManager::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept {
  return _mm_impl->setLight(mode, red, green, blue);
}

/* =======================================================================================
        Handle observers
==========================================================================================
*/

void MeasurementManager::registerClient(MeasurementClient* observer) noexcept {
  return _mm_impl->registerClient(observer);
}

void MeasurementManager::unregisterClient(MeasurementClient* observer) noexcept {
  return _mm_impl->unregisterClient(observer);
}

ManagerState MeasurementManager::getManagerState() const noexcept {
  return _mm_impl->getManagerState();
}

/* =======================================================================================
        Start and stop measurements
==========================================================================================
*/

bool MeasurementManager::measureSome() noexcept{
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

} // namespace manager

} // namespace eduart