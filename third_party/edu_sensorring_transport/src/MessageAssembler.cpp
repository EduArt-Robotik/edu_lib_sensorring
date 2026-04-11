#include "sensorring_transport/MessageAssembler.hpp"

#include <algorithm>

using namespace eduart::transport::protocol;
namespace eduart {
namespace sensorring {
namespace transport {

MessageAssembler::MessageAssembler(std::uint16_t maxPayloadPerFrame)
    : _maxPayloadPerFrame(maxPayloadPerFrame) {
}

std::vector<TransportFrame> MessageAssembler::assemble(Direction direction, std::uint8_t boardAddress, std::uint8_t deviceId, std::uint8_t command, const std::vector<std::uint8_t>& data) const {
  std::vector<TransportFrame> frames;

  if (data.size() <= _maxPayloadPerFrame) {
    // Single frame
    TransportFrame frame;
    frame.direction     = direction;
    frame.boardAddress  = boardAddress;
    frame.deviceId      = deviceId;
    frame.command       = command;
    frame.fragmentType  = fragment::SINGLE;
    frame.fragmentCount = static_cast<std::uint8_t>(data.size() & fragment::COUNT_MASK);
    frame.data          = data;
    frames.push_back(std::move(frame));
    return frames;
  }

  // Multi-frame: calculate total fragments needed
  std::size_t remaining    = data.size();
  std::size_t offset       = 0;
  std::uint8_t totalFrames = 0;
  {
    std::size_t r = remaining;
    while (r > 0) {
      std::size_t chunk = std::min(r, static_cast<std::size_t>(_maxPayloadPerFrame));
      r -= chunk;
      ++totalFrames;
    }
  }

  for (std::uint8_t i = 0; i < totalFrames; ++i) {
    std::size_t chunkSize = std::min(remaining, static_cast<std::size_t>(_maxPayloadPerFrame));

    TransportFrame frame;
    frame.direction    = direction;
    frame.boardAddress = boardAddress;
    frame.deviceId     = deviceId;
    frame.command      = command;
    frame.data.assign(data.begin() + offset, data.begin() + offset + chunkSize);

    if (i == 0) {
      // First frame: count = total number of fragments
      frame.fragmentType  = fragment::FIRST;
      frame.fragmentCount = totalFrames;
    } else if (i == totalFrames - 1) {
      // Last frame: count = valid data bytes in this frame
      frame.fragmentType  = fragment::LAST;
      frame.fragmentCount = static_cast<std::uint8_t>(chunkSize & fragment::COUNT_MASK);
    } else {
      // Middle frame: count = 0
      frame.fragmentType  = fragment::MIDDLE;
      frame.fragmentCount = 0;
    }

    frames.push_back(std::move(frame));
    offset += chunkSize;
    remaining -= chunkSize;
  }

  return frames;
}

} // namespace transport
} // namespace sensorring
} // namespace eduart