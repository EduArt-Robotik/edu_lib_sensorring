#pragma once

#include <memory>

#include "boardmanager/SensorBoardManager.hpp"
#include "interface/ComInterface.hpp"
#include "interface/ComObserver.hpp"
#include "sensors/LedLight.hpp"
#include "sensors/ThermalSensor.hpp"
#include "sensors/TofSensor.hpp"

#include "Parameters.hpp"

namespace eduart {

namespace sensor {

class SensorBoard : public com::ComObserver {
public:
  SensorBoard(SensorBoardParams params, com::ComInterface* interface, std::size_t idx, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds);
  ~SensorBoard();

  SensorBoardType getType() const;
  void setType(SensorBoardType type);

  TofSensor* getTof() const;
  ThermalSensor* getThermal() const;
  LedLight* getLed() const;

  void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  int _idx;
  SensorBoardParams _params;
  SensorBoardType _sensor_type;

  std::unique_ptr<TofSensor> _tof;
  std::unique_ptr<ThermalSensor> _thermal;
  std::unique_ptr<LedLight> _leds;

  com::ComInterface* _interface;
};

}

}