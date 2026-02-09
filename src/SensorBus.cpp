#include "SensorBus.hpp"

#include <chrono>
#include <thread>

#include "interface/can/canprotocol.hpp"
#include "sensorring/device/BaseSensor.hpp"
#include "sensorring/logger/Logger.hpp"
#include "types/EnumerationInformation.hpp"

namespace eduart {

namespace bus {

SensorBus::SensorBus(com::ComInterface* interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec)
    : _interface(interface)
    , _board_vec(std::move(board_vec))
    , _enumeration_flag(false)
    , _enumeration_count(0) {
  if (!_interface) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open com interface");
  }

  subscribeToEndpoint(com::ComEndpoint("broadcast"));
  subscribeToEndpoint(com::ComEndpoint("tof_status"));
  subscribeToEndpoint(com::ComEndpoint("thermal_status"));
  _interface->registerObserver(this);
}

SensorBus::~SensorBus() {
  _interface->unregisterObserver(this);
}

com::ComInterface* SensorBus::getInterface() const {
  return _interface;
}

std::vector<const device::SensorBoard*> SensorBus::getSensorBoards() const {

  std::vector<const device::SensorBoard*> ref_vec;
  for (const auto& sensor : _board_vec) {
    ref_vec.push_back(sensor.get());
  }

  return ref_vec;
}

size_t SensorBus::getSensorCount() const {
  return _board_vec.size();
}

size_t SensorBus::getEnumerationCount() const {
  return _enumeration_count;
}

const std::vector<device::EnumerationInformation>& SensorBus::getEnumerationInfo() const {
  return _enumeration_vec;
}

void SensorBus::setBrs(bool brs_enable) {
  device::SensorBoard::cmdSetBrs(_interface, brs_enable);
}

void SensorBus::resetDevices() {
  device::SensorBoard::cmdReset(_interface);
}

void SensorBus::resetSensorState() {
  for (auto& board : _board_vec) {
    for (device::BaseDevice* device : board->getDevices()) {
      static_cast<device::BaseSensor*>(device)->resetSensorState();
    }
  }
}

int SensorBus::enumerateDevices() {
  _enumeration_vec.clear();
  _enumeration_flag  = true;
  _enumeration_count = 0;

  device::SensorBoard::cmdEnumerateBoards(_interface);

  // wait until all sensors sent their response. 100 ms timeout
  unsigned int watchdog = 0;
  while (_enumeration_count < getSensorCount() && watchdog < 1e3) {
    watchdog += 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // wait a little longer in case there are more sensors than specified
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  _enumeration_flag = false;

  for (auto i = _enumeration_vec.size(); i < _board_vec.size(); i++) {
    auto idx = static_cast<unsigned int>(i + 1);

    // Add configured but unconnected sensors to the enumeration list
    device::EnumerationInformation info;
    info.idx   = idx;
    info.state = device::EnumerationState::ConfiguredNotConnected;
    _enumeration_vec.push_back(std::move(info));

    // Disable sensors that are configured but unconnected
    for (device::BaseDevice* device : _board_vec.at(i)->getDevices()) {
      static_cast<device::BaseSensor*>(device)->setEnable(false);
    }
  }

  return _enumeration_count;
}

void SensorBus::comCallback([[maybe_unused]] const com::ComEndpoint source, [[maybe_unused]] const std::vector<uint8_t>& data) {

  if (source == com::ComEndpoint("broadcast")) { // general sensor board status
    // enumeration message
    if (_enumeration_flag && data.size() == 12 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE) {

      // The bus has to listen to respones to register any boards that are not specified in the configuration
      // Querying the SensorBoards if each has been enumerated can't detect additional boards
      _enumeration_count++;

      auto info  = device::EnumerationInformation::fromBuffer(data);
      info.state = _enumeration_count <= _board_vec.size() ? device::EnumerationState::ConfiguredAndConnected : device::EnumerationState::ConnectedNotConfigured;
      _enumeration_vec.push_back(std::move(info));
    }
  }
}

} // namespace bus

} // namespace eduart