#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>

#include <francor/franklyboot/msg.h>

#include "interface/can/CanInterface.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

class BootloaderProtocol {
public:
  struct RxFrame {
    std::uint32_t can_id{};
    franklyboot::msg::Msg msg{};
  };

  explicit BootloaderProtocol(com::CanInterface& iface, std::chrono::milliseconds timeout);

  void setBroadcastMode();
  void setSpecificNodeMode(std::uint8_t node_id);
  void sendRequest(const franklyboot::msg::Msg& msg);
  std::optional<RxFrame> receive();

  static std::uint8_t nodeFromCanId(std::uint32_t id);
  static std::uint32_t responseCanIdForNode(std::uint8_t node_id);

private:
  static constexpr std::uint32_t CAN_BASE_ID      = 0x781U;
  static constexpr std::uint32_t CAN_BROADCAST_ID = 0x780U;
  static constexpr std::uint32_t CAN_MAX_ID       = 0x7FFU;

  com::CanInterface& _iface;
  std::chrono::milliseconds _timeout;
  com::Subscription _subscription;

  std::mutex _mutex;
  std::condition_variable _cv;
  std::deque<RxFrame> _rx_queue;
  com::CanFilter _active_filter{};

  bool filterMatches(std::uint32_t can_id) const;
  void onRawFrame(const com::RawCanFrame& frame);
};

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
