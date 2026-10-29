#include "sensorring/SensorBus.hpp"

#include "interface/ComInterface.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/board/BoardDiscovery.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensorring {

SensorBus::SensorBus(com::ComInterfaceID interface, std::vector<std::unique_ptr<board::SensorBoard> > board_vec)
    : _interface(com::ComManager::getInstance()->getInterface(interface))
    , _board_vec(std::move(board_vec)) {
  if (!_interface) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open com interface");
  }
}

com::ComInterface* SensorBus::getInterface() const {
  return _interface;
}

std::vector<board::SensorBoard*> SensorBus::getSensorBoards() const {

  std::vector<board::SensorBoard*> ref_vec;
  for (const auto& sensor : _board_vec) {
    ref_vec.push_back(sensor.get());
  }

  return ref_vec;
}

unsigned int SensorBus::getSensorCount() const {
  return static_cast<unsigned int>(_board_vec.size());
}

std::vector<board::EnumerationInformation> SensorBus::queryConnectedDevices(com::ComInterfaceID interface, unsigned int timeout_ms) {
  return board::BoardDiscovery::queryConnectedDevices(interface, timeout_ms);
}

} // namespace sensorring

} // namespace eduart