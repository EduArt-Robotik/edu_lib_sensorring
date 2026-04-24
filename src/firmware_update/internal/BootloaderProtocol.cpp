#include "BootloaderProtocol.hpp"

#include <array>
#include <stdexcept>

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

BootloaderProtocol::BootloaderProtocol(com::CanInterface& iface, std::chrono::milliseconds timeout)
    : _iface(iface)
    , _timeout(timeout)
    , _subscription(_iface.subscribeCanFrames([this](const com::RawCanFrame& frame) {
      onRawFrame(frame);
    })) {
  setBroadcastMode();
}

void BootloaderProtocol::setBroadcastMode() {
  std::lock_guard<std::mutex> lock(_mutex);
  _active_filter = com::CanFilter{ CAN_BROADCAST_ID, CAN_BROADCAST_ID };
  _rx_queue.clear();
}

void BootloaderProtocol::setSpecificNodeMode(std::uint8_t node_id) {
  std::lock_guard<std::mutex> lock(_mutex);
  _active_filter = com::CanFilter{ responseCanIdForNode(node_id), CAN_MAX_ID };
  _rx_queue.clear();
}

void BootloaderProtocol::sendRequest(const franklyboot::msg::Msg& msg) {
  const auto raw = franklyboot::msg::convertMsgToBytes(msg);
  std::vector<std::uint8_t> payload(raw.begin(), raw.end());
  if (!_iface.sendCanFrame(CAN_BROADCAST_ID, payload, false)) {
    throw std::runtime_error("Failed to send bootloader CAN frame");
  }
}

std::optional<BootloaderProtocol::RxFrame> BootloaderProtocol::receive() {
  std::unique_lock<std::mutex> lock(_mutex);
  const bool received = _cv.wait_for(lock, _timeout, [this]() {
    return !_rx_queue.empty();
  });
  if (!received) {
    return std::nullopt;
  }

  RxFrame frame = _rx_queue.front();
  _rx_queue.pop_front();
  return frame;
}

std::uint8_t BootloaderProtocol::nodeFromCanId(std::uint32_t id) {
  if (id < CAN_BASE_ID) {
    throw std::runtime_error("Invalid CAN response ID " + std::to_string(id));
  }
  return static_cast<std::uint8_t>((id - CAN_BASE_ID) / 2U);
}

std::uint32_t BootloaderProtocol::responseCanIdForNode(std::uint8_t node_id) {
  return CAN_BASE_ID + static_cast<std::uint32_t>(node_id) * 2U + 1U;
}

bool BootloaderProtocol::filterMatches(std::uint32_t can_id) const {
  if (_active_filter.can_mask == 0U) {
    return true;
  }
  return (can_id & _active_filter.can_mask) == (_active_filter.can_id & _active_filter.can_mask);
}

void BootloaderProtocol::onRawFrame(const com::RawCanFrame& frame) {
  if (frame.data.size() < 8U) {
    return;
  }

  std::unique_lock<std::mutex> lock(_mutex);
  if (!filterMatches(frame.can_id)) {
    return;
  }

  franklyboot::msg::MsgRaw raw{};
  for (std::size_t i = 0; i < raw.size(); ++i) {
    raw[i] = frame.data[i];
  }

  _rx_queue.push_back(RxFrame{ frame.can_id, franklyboot::msg::convertBytesToMsg(raw) });
  lock.unlock();
  _cv.notify_one();
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
