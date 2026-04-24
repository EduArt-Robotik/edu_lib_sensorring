#include "BootloaderProtocol.hpp"

#include <stdexcept>

#include <sensorring_transport/Protocol.hpp>

#include "sensorring/interface/ComEndpoint.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

namespace {
using namespace eduart::sensorring::transport::protocol;

constexpr std::size_t BOOT_MSG_SIZE = std::tuple_size<franklyboot::msg::MsgRaw>::value;
} // namespace

BootloaderProtocol::BootloaderProtocol(com::ComInterface& interface, std::chrono::milliseconds timeout)
    : _interface(interface)
    , _timeout(timeout)
    , _subscription(_interface.subscribe(
          [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data) {
            onMessage(source, command, data);
          },
          { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::BROADCAST, devbyte::BOARD } })) {
}

void BootloaderProtocol::sendRequest(const franklyboot::msg::Msg& msg) {
  const auto raw = franklyboot::msg::convertMsgToBytes(msg);
  const std::vector<std::uint8_t> payload(raw.begin(), raw.end());

  const com::ComEndpoint target{ com::Direction::Input, com::ComEndpoint::BROADCAST, devbyte::BOARD };
  if (!_interface.send(target, sensor_board::BOOTLOADER_REQUEST, payload)) {
    throw std::runtime_error("Failed to send bootloader request frame");
  }
}

void BootloaderProtocol::sendRequestNoWait(const franklyboot::msg::Msg& msg) {
  // Drop any stale response that might still be in the queue so a subsequent
  // receive() cannot mistake a previous reply for the (never-arriving) echo.
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _rx_queue.clear();
  }
  sendRequest(msg);
}

std::optional<franklyboot::msg::Msg> BootloaderProtocol::receive() {
  std::unique_lock<std::mutex> lock(_mutex);
  const bool received = _cv.wait_for(lock, _timeout, [this]() {
    return !_rx_queue.empty();
  });
  if (!received) {
    return std::nullopt;
  }

  franklyboot::msg::Msg msg = _rx_queue.front();
  _rx_queue.pop_front();
  return msg;
}

void BootloaderProtocol::onMessage(const com::ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data) {
  (void)source;
  if (command != sensor_board::BOOTLOADER_RESPONSE) {
    return;
  }

  // First iteration: exactly one franklyboot message per transport frame.
  if (data.size() != BOOT_MSG_SIZE) {
    return;
  }

  franklyboot::msg::MsgRaw raw{};
  for (std::size_t i = 0; i < raw.size(); ++i) {
    raw[i] = data[i];
  }

  {
    std::lock_guard<std::mutex> lock(_mutex);
    _rx_queue.push_back(franklyboot::msg::convertBytesToMsg(raw));
  }
  _cv.notify_one();
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
