#include "MeasurementManager.hpp"
#include "MeasurementManagerImpl.hpp"
#include "interface/ComManager.hpp"

#include <memory>

namespace eduart {

namespace manager {

MeasurementManager::MeasurementManager(
    std::unique_ptr<MeasurementManagerImpl> mm_impl)
    : _mm_impl(std::move(mm_impl)) {}

MeasurementManager::~MeasurementManager() {}

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

void MeasurementManager::setLight(light::LightMode mode, std::uint8_t red,
                                  std::uint8_t green, std::uint8_t blue) {
  return _mm_impl->setLight(mode, red, green, blue);
}

/* =======================================================================================
        Handle observers
==========================================================================================
*/

void MeasurementManager::registerClient(MeasurementClient *observer) {
  return _mm_impl->registerClient(observer);
}

void MeasurementManager::unregisterClient(MeasurementClient *observer) {
  return _mm_impl->unregisterClient(observer);
}

WorkerState MeasurementManager::getWorkerState() const {
  return _mm_impl->getWorkerState();
}

/* =======================================================================================
        Start and stop measurements
==========================================================================================
*/

bool MeasurementManager::measureSome() { return _mm_impl->measureSome(); }

bool MeasurementManager::startMeasuring() { return _mm_impl->startMeasuring(); }

bool MeasurementManager::stopMeasuring() { return _mm_impl->stopMeasuring(); }

/* =======================================================================================
 Factory Method
==========================================================================================
*/

std::unique_ptr<MeasurementManager>
MeasurementManager::buildManager(ManagerParams params) {
  std::vector<std::unique_ptr<bus::SensorBus>> bus_vec;

  for (const auto &bus_params : params.ring_params.bus_param_vec) {
    auto interface = com::ComManager::getInstance()->createInterface(
        bus_params.interface_name, bus_params.type);

    std::size_t idx = 0;
    std::vector<std::unique_ptr<sensor::SensorBoard>> board_vec;

    for (const auto &board_params : bus_params.board_param_vec) {
      auto tof = std::make_unique<sensor::TofSensor>(board_params.tof_params,
                                                     interface, idx);
      auto thermal = std::make_unique<sensor::ThermalSensor>(
          board_params.thermal_params, interface, idx);
      auto light = std::make_unique<sensor::LedLight>(board_params.led_params);

      board_vec.push_back(std::make_unique<sensor::SensorBoard>(
          board_params, std::move(tof), std::move(thermal), std::move(light)));
      idx++;
    }

    bus_vec.push_back(
        std::make_unique<bus::SensorBus>(interface, std::move(board_vec)));
  }

  auto ring = std::make_unique<ring::SensorRing>(params.ring_params, std::move(bus_vec));
  auto mm_impl = std::make_unique<MeasurementManagerImpl>(params, std::move(ring));
  return std::unique_ptr<MeasurementManager>(new MeasurementManager(std::move(mm_impl)));
}

} // namespace manager

} // namespace eduart