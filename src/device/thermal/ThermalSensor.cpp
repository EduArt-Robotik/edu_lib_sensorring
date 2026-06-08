#include "sensorring/device/thermal/ThermalSensor.hpp"

#include <chrono>

namespace eduart {

namespace sensorring {

namespace device {

const measurement::ThermalMeasurement& ThermalSensor::getLatestMeasurement() const {
  return _latest_measurement;
}

void ThermalSensor::publishMeasurement() {
  if (!deviceEnabled())
    return;
  _latest_measurement.header.timestamp = std::chrono::system_clock::now();
  _thermal_publisher.publish(_latest_measurement);
}

} // namespace device

} // namespace sensorring

} // namespace eduart
