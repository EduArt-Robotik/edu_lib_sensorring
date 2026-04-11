#include "sensorring_transport/MessageReassembler.hpp"

#include <algorithm>

using namespace eduart::transport::protocol;
namespace eduart {
namespace sensorring {
namespace transport {

MessageReassembler::MessageReassembler(DeliveryCallback callback)
    : _callback(std::move(callback)) {
}

void MessageReassembler::processFrame(const TransportFrame& frame) {
  BufferKey key{ frame.boardAddress, frame.deviceId };

  switch (frame.fragmentType) {
  case fragment::SINGLE: {
    // count = valid data bytes; trim any padding
    std::size_t validLen = std::min(static_cast<std::size_t>(frame.fragmentCount), frame.data.size());
    TransportFrame delivered;
    delivered.direction     = frame.direction;
    delivered.boardAddress  = frame.boardAddress;
    delivered.deviceId      = frame.deviceId;
    delivered.command       = frame.command;
    delivered.fragmentType  = fragment::SINGLE;
    delivered.fragmentCount = 0;
    delivered.data.assign(frame.data.begin(), frame.data.begin() + validLen);
    deliver(delivered);
    break;
  }

  case fragment::FIRST: {
    auto it = _buffers.find(key);
    if (it != _buffers.end()) {
      _buffers.erase(it);
    }

    TransferBuffer buf;
    buf.command           = frame.command;
    buf.expectedFragments = frame.fragmentCount;
    buf.receivedFragments = 1;
    buf.direction         = frame.direction;
    buf.boardAddress      = frame.boardAddress;
    buf.deviceId          = frame.deviceId;
    buf.lastActivity      = std::chrono::steady_clock::now();
    buf.data.assign(frame.data.begin(), frame.data.end());
    _buffers.emplace(key, std::move(buf));
    break;
  }

  case fragment::MIDDLE: {
    auto it = _buffers.find(key);
    if (it == _buffers.end())
      return;

    auto& buf = it->second;
    if (buf.command != frame.command) {
      _buffers.erase(it);
      return;
    }

    buf.receivedFragments++;
    if (buf.receivedFragments > buf.expectedFragments) {
      _buffers.erase(it);
      return;
    }

    buf.data.insert(buf.data.end(), frame.data.begin(), frame.data.end());
    buf.lastActivity = std::chrono::steady_clock::now();
    break;
  }

  case fragment::LAST: {
    auto it = _buffers.find(key);
    if (it == _buffers.end())
      return;

    auto& buf = it->second;
    if (buf.command != frame.command) {
      _buffers.erase(it);
      return;
    }

    buf.receivedFragments++;
    if (buf.receivedFragments != buf.expectedFragments) {
      _buffers.erase(it);
      return;
    }

    // count = valid data bytes in last frame; trim padding
    std::size_t validLen = std::min(static_cast<std::size_t>(frame.fragmentCount), frame.data.size());
    buf.data.insert(buf.data.end(), frame.data.begin(), frame.data.begin() + validLen);

    TransportFrame delivered;
    delivered.direction     = buf.direction;
    delivered.boardAddress  = buf.boardAddress;
    delivered.deviceId      = buf.deviceId;
    delivered.command       = buf.command;
    delivered.fragmentType  = fragment::SINGLE;
    delivered.fragmentCount = 0;
    delivered.data          = std::move(buf.data);
    _buffers.erase(it);
    deliver(delivered);
    break;
  }

  default:
    break;
  }
}

void MessageReassembler::purgeStale(std::chrono::milliseconds timeout) {
  auto now = std::chrono::steady_clock::now();
  for (auto it = _buffers.begin(); it != _buffers.end();) {
    if ((now - it->second.lastActivity) > timeout) {
      it = _buffers.erase(it);
    } else {
      ++it;
    }
  }
}

void MessageReassembler::deliver(const TransportFrame& frame) {
  if (_callback) {
    _callback(frame);
  }
}

} // namespace transport
} // namespace sensorring
} // namespace eduart