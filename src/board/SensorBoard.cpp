#include "sensorring/board/SensorBoard.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "interface/ComManager.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::sensor_board;

namespace eduart {

namespace sensorring {

namespace board {

SensorBoard::SensorBoard(SensorBoardParams params, com::ComInterfaceID interface, unsigned int idx, std::vector<std::unique_ptr<device::Device> > devices)
    : _idx(idx)
    , _params(params)
    , _enum_info()
    , _device_vec(std::move(devices))
    , _got_update(false)
    , _interface(com::ComManager::getInstance()->getInterface(interface)) {

  for (auto& device : _device_vec) {
    device->setBoardContext(&_params);
  }

  _com_subscription = _interface->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
  },
      { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } });

  // configure();
}

SensorBoard::~SensorBoard() {
  // _com_subscription auto-cancels via RAII.
}

bool SensorBoard::isEnumerated() const {
  return _enum_info.type != SensorBoardType::Undefined && _enum_info.hash.hash != 0;
}

const EnumerationInformation& SensorBoard::getEnumInfo() const {
  return _enum_info;
}

SensorBoardType SensorBoard::getBoardType() const {
  return _params.board_type;
}

std::vector<device::Device*> SensorBoard::getDevices() const {
  std::vector<device::Device*> devices;
  devices.reserve(_device_vec.size());
  for (auto& device : _device_vec) {
    devices.push_back(device.get());
  }
  return devices;
}

bool SensorBoard::configure() {
  auto success = setOrientation(_params.orientation);
  if (!success) {
    return false;
  }

  for (const auto& dev : _device_vec) {
    success &= dev->configure();
    if (!success) {
      return false;
    }
  }

  return success;
}

bool SensorBoard::setOrientation(Orientation orientation) {
  bool success = _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_idx + 1), devbyte::BOARD }, PARAMETER_SET_ORIENTATION, { static_cast<std::uint8_t>(orientation) });

  Orientation current_orientation;
  success &= getOrientation(current_orientation);
  success &= (current_orientation == orientation);

  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set orientation on board " + std::to_string(_idx) + " to " + toString(orientation));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Failed to set orientation on board " + std::to_string(_idx) + " to " + toString(orientation) + ".");
    // return false;
  }

  return true;
}

bool SensorBoard::getOrientation(Orientation& orientation) {
  _got_update = false;

  _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_idx + 1), devbyte::BOARD }, PARAMETER_GET_ORIENTATION, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(GET_PARAMETER_SLEEP);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got orientation update on board " + std::to_string(_idx) + ": orientation " + toString(_params.orientation));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get orientation on board " + std::to_string(_idx));
    return false;
  }

  orientation = _params.orientation;
  return true;
}

void SensorBoard::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {

  switch (command) {
  case sensor_board::ACTIVE_DEVICE_RESPONSE:
    if (data.size() >= 11 && (data.at(0) == _idx)) {
      RecursiveLock lock(_com_mutex);
      if (_enum_info.isUndefined()) {
        _enum_info       = EnumerationInformation::fromBuffer(data);
        _enum_info.state = ConnectionState::Connected;

        // Re-subscribe to board-specific messages now that the board type is known.
        _com_subscription = _interface->subscribe(
            [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
              this->comCallback(source, command, data);
        },
            { com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(_idx + 1), devbyte::BOARD } });
      }
    }
    return;
  case PARAMETER_GET_ORIENTATION: {
    if (data.size() >= 1) {
      RecursiveLock lock(_com_mutex);
      _params.orientation = static_cast<Orientation>(data[0]);
      _got_update         = true;
    }
    return;
  }
  }
}

} // namespace board

} // namespace sensorring

} // namespace eduart