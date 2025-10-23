#include "SensorBoard.hpp"

#include "boardmanager/SensorBoardManager.hpp"
#include "interface/ComEndpoints.hpp"
#include "interface/can/canprotocol.hpp"

#include "Math.hpp"

namespace eduart {

namespace sensor {

SensorBoard::SensorBoard(
    SensorBoardParams params, com::ComInterface* interface, std::size_t idx, std::unique_ptr<TofSensor> tof,
    std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds)
    : _idx(idx)
    , _interface(interface)
    , _params{ params }
    , _board_type(SensorBoardType::Undefined)
    , _fw_rev({ 0, 0, 0 })
    , _tof(std::move(tof))
    , _thermal(std::move(thermal))
    , _leds(std::move(leds)) {

  addEndpoint(com::ComEndpoint("broadcast"));
  _interface->registerObserver(this);
}

SensorBoard::~SensorBoard() {
}

SensorBoardType SensorBoard::getType() const {
  return _board_type;
}

FwRevision SensorBoard::getFwRevision() const {
  return _fw_rev;
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
  // ToDo: Eliminate offset of index
  if (data.size() == 3 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE && (data.at(1) == _idx + 1)) {
    if (_board_type == SensorBoardType::Undefined) {
      _board_type = static_cast<SensorBoardType>(data.at(2));

      const auto board_infos = SensorBoardManager::getSensorBoardInfo(_board_type);

      const auto tof_translation = _params.translation + board_infos.tof.board_center_translation_offset;
      const auto tof_rotation    = math::Vector3::eulerDegreesFromRotationMatrix(
          math::Matrix3::rotMatrixFromEulerDegrees(_params.rotation)
          * math::Matrix3::rotMatrixFromEulerDegrees(board_infos.tof.board_center_rotation_offset));
      _tof->setPose(tof_translation, tof_rotation);

      const auto thermal_translation = _params.translation + board_infos.thermal.board_center_translation_offset;
      const auto thermal_rotation    = math::Vector3::eulerDegreesFromRotationMatrix(
          math::Matrix3::rotMatrixFromEulerDegrees(_params.rotation)
          * math::Matrix3::rotMatrixFromEulerDegrees(board_infos.thermal.board_center_rotation_offset));
      _thermal->setPose(thermal_translation, thermal_rotation);
    }
  }

  if (data.size() == 8 && data.at(0) == _idx + 1) {
    _fw_rev  = { data.at(1), data.at(2), data.at(3) };
    _fw_hash = (static_cast<std::uint32_t>(data.at(4)) << 24) | (static_cast<std::uint32_t>(data.at(5)) << 16)
               | (static_cast<std::uint32_t>(data.at(6)) << 8) | (static_cast<std::uint32_t>(data.at(7)) << 0);
  }
}

} // namespace sensor

} // namespace eduart