#include "sensorring/SensorBus.hpp"

#include <chrono>
#include <thread>

#include "device/DeviceEnumerator.hpp"
#include "interface/ComInterface.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace bus {

SensorBus::SensorBus(com::ComInterfaceID interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec)
    : _interface(com::ComManager::getInstance()->getInterface(interface))
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