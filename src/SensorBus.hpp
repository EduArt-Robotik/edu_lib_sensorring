#pragma once

#include <memory>
#include <vector>

#include "interface/ComInterface.hpp"
#include "interface/ComObserver.hpp"

#include "SensorBoard.hpp"

namespace eduart {

namespace bus {

class SensorBus : public com::ComObserver {
public:
  SensorBus(com::ComInterface* interface, std::vector<std::unique_ptr<sensor::SensorBoard> > board_vec);
  ~SensorBus();

  size_t getSensorCount() const;
  size_t getEnumerationCount() const;
  const std::vector<sensor::EnumerationInformation>& getEnumerationInfo() const;

  com::ComInterface* getInterface() const;

  std::vector<const sensor::SensorBoard*> getSensorBoards() const;
  bool isTofEnabled(int idx) const;
  bool isThermalEnabled(int idx) const;

  bool allTofMeasurementsReady() const;
  bool allTofMeasurementsReady(unsigned int& ready_sensors_count) const;
  bool allThermalMeasurementsReady() const;
  bool allThermalMeasurementsReady(unsigned int& ready_sensors_count) const;
  bool allTofDataTransmissionsComplete() const;
  bool allTofDataTransmissionsComplete(unsigned int& ready_sensors_count) const;
  bool allThermalDataTransmissionsComplete() const;
  bool allThermalDataTransmissionsComplete(unsigned int& ready_sensors_count) const;
  bool allEEPROMTransmissionsComplete() const;

  void resetDevices();
  void resetSensorState();
  int enumerateDevices();
  void setBrs(bool brs_enable);
  void syncLight();
  void setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);
  void requestEEPROM();
  void requestTofMeasurement();
  void fetchTofMeasurement();
  void requestThermalMeasurement();
  void fetchThermalMeasurement();

  bool stopThermalCalibration();
  bool startThermalCalibration(unsigned int window);

  void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  com::ComInterface* _interface;
  std::vector<sensor::EnumerationInformation> _enumeration_vec;
  std::vector<std::unique_ptr<sensor::SensorBoard> > _board_vec;
  
  std::atomic<bool> _enumeration_flag;
  std::atomic<unsigned int> _enumeration_count;

  unsigned int _active_tof_sensors;
  unsigned int _active_thermal_sensors;
  unsigned int _tof_measurement_count;
  unsigned int _thermal_measurement_count;
};

} // namespace bus

} // namespace eduart