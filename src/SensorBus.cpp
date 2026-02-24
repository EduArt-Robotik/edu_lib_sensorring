#include "sensorring/SensorBus.hpp"

#include <chrono>
#include <thread>

#include "device/DeviceEnumerator.hpp"
#include "interface/ComInterface.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/device/BaseSensor.hpp"
#include "sensorring/device/EnumerationInformation.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace bus {

SensorBus::SensorBus(com::ComInterfaceID interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec)
    : _interface(com::ComManager::getInstance()->getInterface(interface))
    , _enumeration_flag(false)
    , _enumeration_vec()
    , _board_vec(std::move(board_vec)) {
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

std::vector<device::SensorBoard*> SensorBus::getSensorBoards() const {

  std::vector<device::SensorBoard*> ref_vec;
  for (const auto& sensor : _board_vec) {
    ref_vec.push_back(sensor.get());
  }

  return ref_vec;
}

size_t SensorBus::getSensorCount() const {
  return _board_vec.size();
}

void SensorBus::setBrs(bool brs_enable) {
  device::SensorBoard::cmdSetBrs(_interface->getID(), brs_enable);
}

bool SensorBus::verifyTopology() {
  enumerateDevices();

  if (_enumeration_vec.size() != _board_vec.size()) {
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Warning,
        "Mismatch while verifying the topology on interface " + _interface->getID().name + ": " + std::to_string(_board_vec.size()) + "devices are configured but " + std::to_string(_enumeration_vec.size()) + " are connected!");
    return false;
  }

  std::size_t idx = 0;
  for (const auto& enum_device : _enumeration_vec) {
    if (enum_device.type != _board_vec.at(idx)->getBoardType()) {
      logger::Logger::getInstance()->log(
          logger::LogVerbosity::Warning, "Mismatch while verifying the topology on interface " + _interface->getID().name + ": Device " + std::to_string(idx) + " is configured as " + toString(_board_vec.at(idx)->getBoardType()) + " but a "
                                             + toString(enum_device.type) + " is connected!");
      return false;
    }
    idx++;
  }

  return true;
}

std::vector<device::EnumerationInformation> SensorBus::enumerateDevices() {

  _enumeration_vec = queryConnectedDevices(_interface->getID());

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

  _enumeration_flag = true;
  return _enumeration_vec;
}

const std::vector<device::EnumerationInformation>& SensorBus::getLatestEnumerationResult() {
  if (!_enumeration_flag) {
    enumerateDevices();
  }
  return _enumeration_vec;
}

std::vector<device::EnumerationInformation> SensorBus::queryConnectedDevices(com::ComInterfaceID interface) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (!iface) {
    return {};
  }

  device::DeviceEnumerator enumerator(iface);
  enumerator.startEnumeration();
  std::this_thread::sleep_for(ENUMERATION_TIMEOUT);
  return enumerator.getResult();
}

void SensorBus::comCallback([[maybe_unused]] const com::ComEndpoint source, [[maybe_unused]] const std::vector<uint8_t>& data) {
}

} // namespace bus

} // namespace eduart