#include "SensorBoard.hpp"

#include "boardmanager/SensorBoardManager.hpp"
#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/logger/Logger.hpp"
#include "sensorring/math/Math.hpp"

namespace eduart {

namespace device {

SensorBoard::SensorBoard(SensorBoardParams params, com::ComInterface* interface, unsigned int idx, std::unique_ptr<VL53L8CX_Device> tof, std::unique_ptr<HTPA32_Device> thermal, std::unique_ptr<WS2812b_Device> leds)
    : _idx(idx)
    , _interface(interface)
    , _params{ params }
    , _enum_info() {

  _device_vec.push_back(std::move(tof));
  _device_vec.push_back(std::move(thermal));
  _device_vec.push_back(std::move(leds));

  _interface->addSensorBoardEndpoint();

  subscribeToEndpoint(com::ComEndpoint("broadcast"));
  _interface->registerObserver(this);
}

SensorBoard::~SensorBoard() {
  _interface->unregisterObserver(this);
}

bool SensorBoard::isEnumerated() const {
  return _enum_info.type != SensorBoardType::Undefined && _enum_info.hash.hash != 0;
}

const EnumerationInformation& SensorBoard::getEnumInfo() const {
  return _enum_info;
}

std::vector<BaseDevice*> SensorBoard::getDevices() const {
  std::vector<BaseDevice*> devices;
  devices.reserve(_device_vec.size());
  for (auto& device : _device_vec) {
    devices.push_back(device.get());
  }
  return devices;
}

VL53L8CX_Device* SensorBoard::getTof() const {
  LockGuard lock(_com_mutex);

  if (auto tof = dynamic_cast<VL53L8CX_Device*>(_device_vec[0].get())) {
    return tof;
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "VL53L8CX_Device not found");
    return nullptr;
  }
}

HTPA32_Device* SensorBoard::getThermal() const {
  LockGuard lock(_com_mutex);
  if (auto thermal = dynamic_cast<HTPA32_Device*>(_device_vec[1].get())) {
    return thermal;
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "HTPA32_Device not found");
    return nullptr;
  }
}

WS2812b_Device* SensorBoard::getLed() const {
  LockGuard lock(_com_mutex);
  if (auto led = dynamic_cast<WS2812b_Device*>(_device_vec[2].get())) {
    return led;
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "WS2812b_Device not found");
    return nullptr;
  }
}

bool SensorBoard::resetBoards() {
  bool success                = true;
  std::vector<uint8_t> tx_buf = { CMD_HARD_RESET };
  for (auto& iface : com::ComManager::getInstance()->getInterfaces()) {
    success &= iface->send(com::ComEndpoint("broadcast"), tx_buf);
  }
  return success;
}

void SensorBoard::cmdSetBrs(com::ComInterface* interface, bool enable) {
  std::vector<uint8_t> tx_buf = { CMD_SET_BRS, 0xFF, 0xFF, enable ? std::uint8_t(0x01) : std::uint8_t(0x00) };
  interface->send(com::ComEndpoint("broadcast"), tx_buf);
}

void SensorBoard::cmdEnumerateBoards(com::ComInterface* interface) {
  std::vector<uint8_t> tx_buf_enumeration = { CMD_ACTIVE_DEVICE_QUERY, CMD_ACTIVE_DEVICE_QUERY };
  interface->send(com::ComEndpoint("broadcast"), tx_buf_enumeration);
}

void SensorBoard::comCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  // ToDo: Eliminate offset of index
  if (data.size() == 12 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE && (data.at(1) == _idx + 1)) {

    LockGuard lock(_com_mutex);

    if (_enum_info.isUndefined()) {
      _enum_info       = EnumerationInformation::fromBuffer(data);
      _enum_info.state = EnumerationState::ConfiguredAndConnected;

      const auto board_infos = SensorBoardManager::getSensorBoardInfo(_enum_info.type);

      const auto tof_translation = _params.translation + board_infos.tof.board_center_translation_offset;
      const auto tof_rotation    = math::eulerDegreesFromRotationMatrix(math::rotMatrixFromEulerDegrees(_params.rotation) * math::rotMatrixFromEulerDegrees(board_infos.tof.board_center_rotation_offset));
      getTof()->setPose(tof_translation, tof_rotation);

      const auto thermal_translation = _params.translation + board_infos.thermal.board_center_translation_offset;
      const auto thermal_rotation    = math::eulerDegreesFromRotationMatrix(math::rotMatrixFromEulerDegrees(_params.rotation) * math::rotMatrixFromEulerDegrees(board_infos.thermal.board_center_rotation_offset));
      getThermal()->setPose(thermal_translation, thermal_rotation);
    }
  }
}

} // namespace device

} // namespace eduart