#include "sensorring/SensorBus.hpp"

#include <chrono>
#include <thread>

#include "interface/ComInterface.hpp"
#include "interface/ComManager.hpp"
#include "interface/can/canprotocol.hpp"
#include "sensorring/device/BaseSensor.hpp"
#include "sensorring/device/EnumerationInformation.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace bus {

namespace {

class EnumerationCollector : public com::ComObserver {
public:
  explicit EnumerationCollector(com::ComInterface* interface, std::vector<device::EnumerationInformation>& out)
      : _interface(interface)
      , _out(out) {
    subscribeToEndpoint(com::ComEndpoint("broadcast"));
    _interface->registerObserver(this);
  }

  ~EnumerationCollector() { _interface->unregisterObserver(this); }

  void trigger() { device::SensorBoard::cmdEnumerateBoards(_interface->getID()); }

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override {
    (void)source;
    if (data.size() == 12 && data.at(0) == CMD_ACTIVE_DEVICE_RESPONSE) {
      auto info  = device::EnumerationInformation::fromBuffer(data);
      info.state = device::EnumerationState::ConfiguredAndConnected;
      _out.push_back(std::move(info));
    }
  }

private:
  com::ComInterface* _interface;
  std::vector<device::EnumerationInformation>& _out;
};

} // namespace

SensorBus::SensorBus(com::ComInterfaceID interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec)
    : _interface(com::ComManager::getInstance()->getInterface(interface))
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

size_t SensorBus::getEnumerationCount() const {
  return _enumeration_count;
}

const std::vector<device::EnumerationInformation>& SensorBus::getEnumerationInfo() const {
  return _enumeration_vec;
}

void SensorBus::setBrs(bool brs_enable) {
  device::SensorBoard::cmdSetBrs(_interface->getID(), brs_enable);
}

int SensorBus::enumerateDevices() {
  _enumeration_vec.clear();
  _enumeration_flag  = true;
  _enumeration_count = 0;

  device::SensorBoard::cmdEnumerateBoards(_interface->getID());

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

std::vector<device::EnumerationInformation> SensorBus::enumerateInterface(com::ComInterfaceID interface) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (!iface) {
    return {};
  }

  std::vector<device::EnumerationInformation> result;
  EnumerationCollector collector(iface, result);
  collector.trigger();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  return result;
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