#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "interface/ComInterface.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

class BootloaderActivator {
public:
  explicit BootloaderActivator(com::ComInterface& interface, std::chrono::milliseconds ack_timeout);

  bool enterBootloader(std::uint8_t board_address);

private:
  com::ComInterface& _interface;
  std::chrono::milliseconds _ack_timeout;
  com::Subscription _subscription;

  std::mutex _mutex;
  std::condition_variable _cv;
  std::uint8_t _awaited_board_address{ 0U };
  bool _ack_received{ false };

  void onMessage(const com::ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data);
};

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
