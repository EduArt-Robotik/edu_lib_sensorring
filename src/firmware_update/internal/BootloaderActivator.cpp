#include "BootloaderActivator.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "sensorring/interface/ComEndpoint.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

using namespace eduart::sensorring::transport::protocol;

BootloaderActivator::BootloaderActivator(com::ComInterface& interface, std::chrono::milliseconds ack_timeout)
    : _interface(interface)
    , _ack_timeout(ack_timeout)
    , _subscription(_interface.subscribe(
          [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data) {
            onMessage(source, command, data);
          },
          { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } })) {
}

bool BootloaderActivator::enterBootloader(std::uint8_t board_address) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _awaited_board_address = board_address;
    _ack_received          = false;
  }

  if (!_interface.send(com::ComEndpoint{ com::Direction::Input, board_address, devbyte::BOARD }, sensor_board::BOOTLOADER_START, {})) {
    return false;
  }

  std::unique_lock<std::mutex> lock(_mutex);
  return _cv.wait_for(lock, _ack_timeout, [this]() {
    return _ack_received;
  });
}

void BootloaderActivator::onMessage(const com::ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data) {
  (void)data;
  if (command != sensor_board::BOOTLOADER_START_ACK) {
    return;
  }

  std::lock_guard<std::mutex> lock(_mutex);
  if (source.boardAddress != _awaited_board_address) {
    return;
  }

  _ack_received = true;
  _cv.notify_all();
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
