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

ManagerState MeasurementManager::getManagerState() const noexcept {
  return _mm_impl->getManagerState();
}

subscription::Subscription MeasurementManager::subscribeToStateChanges(std::function<void(const ManagerState state)> callback) {
  return _mm_impl->subscribeToStateChanges(std::move(callback));
}

subscription::Subscription MeasurementManager::subscribeToDeviceGroup(device::DeviceType key, std::function<void(const device::DeviceGroup&)> callback) {
  return _mm_impl->subscribeToDeviceGroup(key, std::move(callback));
}

void MeasurementManager::unsubscribe(subscription::SubscriberToken token) {
  return _mm_impl->unsubscribe(token);
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