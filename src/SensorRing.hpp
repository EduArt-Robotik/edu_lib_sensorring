#pragma once

#include <memory>
#include <vector>

#include "Parameters.hpp"
#include "SensorBus.hpp"

namespace eduart {

namespace ring {

class SensorRing {

public:
  SensorRing(RingParams params, std::vector<std::unique_ptr<bus::SensorBus> > bus_vec);
  ~SensorRing();

  std::vector<const bus::SensorBus*> getInterfaces() const;

  void setBRS(bool brs_enable);
  void syncLight();
  void setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);
  void resetDevices();
  bool enumerateDevices();
  bool getEEPROM();
  void requestTofMeasurement();
  void fetchTofMeasurement();
  void requestThermalMeasurement();
  void fetchThermalMeasurement();
  bool stopThermalCalibration();
  bool startThermalCalibration(size_t window);

  bool waitForAllTofMeasurementsReady() const;
  bool waitForAllTofDataTransmissionsComplete() const;
  bool waitForAllThermalMeasurementsReady() const;
  bool waitForAllThermalDataTransmissionsComplete() const;

private:
  const RingParams _params;
  std::vector<std::unique_ptr<bus::SensorBus> > _bus_vec;
};

} // namespace ring

} // namespace eduart