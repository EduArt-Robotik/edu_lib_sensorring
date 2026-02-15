#include "sensorring/manager/MeasurementManager.hpp"

#include "manager/MeasurementManagerImpl.hpp"

namespace eduart {

namespace manager {

MeasurementManager::MeasurementManager(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring)
    : _mm_impl(std::make_unique<MeasurementManagerImpl>(params, std::move(sensor_ring))) {
}

MeasurementManager::~MeasurementManager() noexcept {
}

ManagerParams MeasurementManager::getParams() const noexcept {
  return _mm_impl->getParams();
}

ring::SensorRing* MeasurementManager::getSensorRing() const noexcept {
  return _mm_impl->getSensorRing();
}

void MeasurementManager::enqueueExtraAction(std::function<void()> action) {
  return _mm_impl->enqueueExtraAction(std::move(action));
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

ManagerState MeasurementManager::getManagerState() const noexcept {
  return _mm_impl->getManagerState();
}

SubscriptionToken MeasurementManager::subscribeToDeviceGroup(DeviceGroupKey key, std::function<void(const device::DeviceGroup&)> callback) {
  return _mm_impl->subscribeToDeviceGroup(key, callback);
}
  
void MeasurementManager::unsubscribeFromDeviceGroup(SubscriptionToken token) {
  return _mm_impl->unsubscribeFromDeviceGroup(token);
}

/* =======================================================================================
        Start and stop measurements
==========================================================================================
*/

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

} // namespace manager

} // namespace eduart