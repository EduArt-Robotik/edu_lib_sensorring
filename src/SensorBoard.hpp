#pragma once

#include <memory>

#include "sensors/LedLight.hpp"
#include "sensors/ThermalSensor.hpp"
#include "sensors/TofSensor.hpp"

#include "Parameters.hpp"

namespace eduart {

namespace sensor {

enum class SensorBoardType {
  headlight,
  taillight,
  sidepanel,
  minipanel,
  unknown
};

class SensorBoard {
public:
  SensorBoard(SensorBoardParams params, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds);
  ~SensorBoard();

  inline SensorBoardType getType() const;
  inline void setType(SensorBoardType type);

  TofSensor* getTof() const;
  ThermalSensor* getThermal() const;
  LedLight* getLed() const;

private:
  SensorBoardParams _params;
  SensorBoardType _sensor_type;

  std::unique_ptr<TofSensor> _tof;
  std::unique_ptr<ThermalSensor> _thermal;
  std::unique_ptr<LedLight> _leds;
};

inline SensorBoardType SensorBoard::getType() const {
  return _sensor_type;
}

inline void SensorBoard::setType(SensorBoardType type) {
  _sensor_type = type;
}

inline TofSensor* SensorBoard::getTof() const {
  return _tof.get();
}

inline ThermalSensor* SensorBoard::getThermal() const {
  return _thermal.get();
}

inline LedLight* SensorBoard::getLed() const {
  return _leds.get();
}

}

}