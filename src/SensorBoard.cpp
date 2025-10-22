#include "SensorBoard.hpp"

#include "interface/ComEndpoints.hpp"
#include "interface/can/canprotocol.hpp"

namespace eduart {

namespace sensor {

SensorBoard::SensorBoard(SensorBoardParams params, com::ComInterface* interface, std::size_t idx, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds)
    : _idx(idx)
    , _params{ params }
    , _sensor_type(SensorBoardType::Unknown)
    , _tof(std::move(tof))
    , _thermal(std::move(thermal))
    , _leds(std::move(leds))
    , _interface(interface) {

  addEndpoint(com::ComEndpoint("broadcast"));
  _interface->registerObserver(this);
}

SensorBoard::~SensorBoard() {
}

SensorBoardType SensorBoard::getType() const {
  return _sensor_type;
}

void SensorBoard::setType(SensorBoardType type) {
  _sensor_type = type;
}

TofSensor* SensorBoard::getTof() const {
  return _tof.get();
}

ThermalSensor* SensorBoard::getThermal() const {
  return _thermal.get();
}

LedLight* SensorBoard::getLed() const {
  return _leds.get();
}

void SensorBoard::notify([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  if (data.size() == 3 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE && (data.at(1) == _idx + 1)) { // ToDo: Eliminate offset
    if (_sensor_type == SensorBoardType::Unknown)
      _sensor_type = static_cast<SensorBoardType>(data.at(2));
  }
}

}

}