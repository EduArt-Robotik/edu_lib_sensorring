#include "MeasurementManager.hpp"

#include <memory>

#include "MeasurementManagerImpl.hpp"

namespace eduart {

namespace manager {

MeasurementManager::MeasurementManager(ManagerParams params)
    : _mm_impl(std::make_unique<MeasurementManagerImpl>(params)) {
}

MeasurementManager::~MeasurementManager() {
}

void MeasurementManager::enableTofMeasurement(bool state) {
  return _mm_impl->enableTofMeasurement(state);
}

void MeasurementManager::enableThermalMeasurement(bool state) {
  return _mm_impl->enableThermalMeasurement(state);
}

ManagerParams MeasurementManager::getParams() const {
  return _mm_impl->getParams();
}

std::string MeasurementManager::printTopology() const {
  return _mm_impl->printTopology();
}

bool MeasurementManager::stopThermalCalibration() {
  return _mm_impl->stopThermalCalibration();
}

bool MeasurementManager::startThermalCalibration(std::size_t window) {
  return _mm_impl->startThermalCalibration(window);
}

void MeasurementManager::setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  return _mm_impl->setLight(mode, red, green, blue);
}

/* =======================================================================================
        Handle observers
==========================================================================================
*/

void MeasurementManager::registerClient(MeasurementClient* observer) {
  return _mm_impl->registerClient(observer);
}

void MeasurementManager::unregisterClient(MeasurementClient* observer) {
  return _mm_impl->unregisterClient(observer);
}

ManagerState MeasurementManager::getManagerState() const {
  return _mm_impl->getManagerState();
}

/* =======================================================================================
        Start and stop measurements
==========================================================================================
*/

bool MeasurementManager::measureSome() {
  return _mm_impl->measureSome();
}

bool MeasurementManager::startMeasuring() {
  return _mm_impl->startMeasuring();
}

bool MeasurementManager::stopMeasuring() {
  return _mm_impl->stopMeasuring();
}

bool MeasurementManager::isMeasuring(){
  return _mm_impl->isMeasuring();
}

} // namespace manager

} // namespace eduart