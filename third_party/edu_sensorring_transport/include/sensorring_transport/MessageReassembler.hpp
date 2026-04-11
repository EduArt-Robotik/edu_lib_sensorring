/**
 * @file   MessageReassembler.hpp
 * @brief  MessageReassembler: reassembles TransportFrames into complete messages.
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include "sensorring_transport/Protocol.hpp"

namespace eduart {
namespace sensorring {
namespace transport {

/**
 * @class MessageReassembler
 * @brief Reassembles fragmented TransportFrames into complete messages.
 *
 * Single-frame messages are delivered immediately. Multi-frame transfers are
 * buffered until the last fragment arrives. Stale incomplete transfers can
 * be purged via purgeStale().
 */
class MessageReassembler {
public:
  /**
   * @brief Callback signature for delivering complete reassembled messages.
   *
   * @param frame  A TransportFrame with fragmentType == FRAG_SINGLE, containing the complete payload.
   *               direction, boardAddress, deviceId, and command are set from the original frames.
   */
  using DeliveryCallback = std::function<void(const TransportFrame& frame)>;

  explicit MessageReassembler(DeliveryCallback callback);

  /**
   * @brief Process one incoming TransportFrame.
   *
   * @param frame  A decoded TransportFrame from the transport codec.
   */
  void processFrame(const TransportFrame& frame);

  /**
   * @brief Discard incomplete transfers idle for longer than the timeout.
   */
  void purgeStale(std::chrono::milliseconds timeout);

private:
  struct TransferBuffer {
    std::uint8_t command;
    std::uint8_t expectedFragments;
    std::uint8_t receivedFragments;
    Direction direction;
    std::uint8_t boardAddress;
    std::uint8_t deviceId;
    std::vector<std::uint8_t> data;
    std::chrono::steady_clock::time_point lastActivity;
  };

  struct BufferKey {
    std::uint8_t boardAddress;
    std::uint8_t deviceId;

    bool operator==(const BufferKey& o) const { return boardAddress == o.boardAddress && deviceId == o.deviceId; }
  };

  struct BufferKeyHash {
    std::size_t operator()(const BufferKey& k) const { return std::hash<std::uint16_t>{}((static_cast<std::uint16_t>(k.boardAddress) << 8) | k.deviceId); }
  };

  void deliver(const TransportFrame& frame);

  DeliveryCallback _callback;
  std::unordered_map<BufferKey, TransferBuffer, BufferKeyHash> _buffers;
};

} // namespace transport
} // namespace sensorring
} // namespace eduart