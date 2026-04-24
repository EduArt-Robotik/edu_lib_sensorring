#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <vector>

#include <francor/franklyboot/msg.h>

#include "interface/ComInterface.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

/**
 * @brief Host-side counterpart of the on-device franklyboot handler.
 *
 * Wraps a generic @ref com::ComInterface and exchanges a single
 * franklyboot::msg::Msg per transport frame using the
 * sensor_board::BOOTLOADER_REQUEST / BOOTLOADER_RESPONSE commands.
 *
 * The firmware broadcasts its responses (boardAddress == 0x00). We therefore
 * assume at most one board is ever in bootloader mode at a time -- which is
 * enforced upstream in hardware by the frontier update scheme.
 */
class BootloaderProtocol {
public:
  explicit BootloaderProtocol(com::ComInterface& interface, std::chrono::milliseconds timeout);

  /// Send one franklyboot request as a SINGLE transport frame on broadcast.
  void sendRequest(const franklyboot::msg::Msg& msg);

  /// Block until a matching BOOTLOADER_RESPONSE arrives or the timeout expires.
  std::optional<franklyboot::msg::Msg> receive();

private:
  com::ComInterface& _interface;
  std::chrono::milliseconds _timeout;
  com::Subscription _subscription;

  std::mutex _mutex;
  std::condition_variable _cv;
  std::deque<franklyboot::msg::Msg> _rx_queue;

  void onMessage(const com::ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data);
};

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
