#include "BootloaderProtocol.hpp"

#include <sensorring_transport/Protocol.hpp>
#include <stdexcept>

#include "sensorring/interface/ComEndpoint.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

namespace {
using namespace eduart::sensorring::transport::protocol;

constexpr std::size_t BOOT_MSG_SIZE      = std::tuple_size<franklyboot::msg::MsgRaw>::value;
constexpr std::size_t MAX_MSGS_PER_FRAME = 7U;
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
  sendRequests({ msg });
}

void BootloaderProtocol::sendRequests(const std::vector<franklyboot::msg::Msg>& msgs) {
  if (msgs.empty()) {
    throw std::runtime_error("Cannot send empty bootloader request batch");
  }
  if (msgs.size() > MAX_MSGS_PER_FRAME) {
    throw std::runtime_error("Bootloader request batch exceeds transport frame capacity");
  }

  std::vector<std::uint8_t> payload;
  payload.reserve(msgs.size() * BOOT_MSG_SIZE);
  for (const auto& msg : msgs) {
    const auto raw = franklyboot::msg::convertMsgToBytes(msg);
    payload.insert(payload.end(), raw.begin(), raw.end());
  }

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

  if (data.empty() || (data.size() % BOOT_MSG_SIZE) != 0U) {
    return;
  }
  const std::size_t msg_count = data.size() / BOOT_MSG_SIZE;
  if (msg_count > MAX_MSGS_PER_FRAME) {
    return;
  }

  std::vector<franklyboot::msg::Msg> parsed_msgs;
  parsed_msgs.reserve(msg_count);
  for (std::size_t msg_idx = 0; msg_idx < msg_count; ++msg_idx) {
    franklyboot::msg::MsgRaw raw{};
    const std::size_t offset = msg_idx * BOOT_MSG_SIZE;
    for (std::size_t i = 0; i < raw.size(); ++i) {
      raw[i] = data[offset + i];
    }
    parsed_msgs.push_back(franklyboot::msg::convertBytesToMsg(raw));
  }
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _rx_queue.insert(_rx_queue.end(), parsed_msgs.begin(), parsed_msgs.end());
  }
  _cv.notify_all();
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
