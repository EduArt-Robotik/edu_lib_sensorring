#include "SensorBoard.hpp"

namespace eduart {

namespace sensor {

SensorBoard::SensorBoard(SensorBoardParams params, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds)
    : _params{ params }
    , _sensor_type(SensorBoardType::unknown) {
  _tof     = std::move(tof);
  _thermal = std::move(thermal);
  _leds    = std::move(leds);
}

SensorBoard::~SensorBoard() {
}

}

}