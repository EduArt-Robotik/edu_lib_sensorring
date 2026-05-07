#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "sensorring/interface/ComInterfaceID.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {

struct UpdateConfig {
  std::chrono::milliseconds can_timeout{ 500 };
  std::chrono::milliseconds bootloader_start_ack_timeout{ 1200 };
  std::chrono::milliseconds settle_delay_after_flash{ 500 };
  std::chrono::milliseconds bootloader_detect_retry_delay{ 200 };
  unsigned int bootloader_detect_retries{ 5 };
  unsigned int no_progress_cycles_before_done{ 3 };
};

using LogCallback = std::function<void(const std::string&)>;

class FirmwareUpdater {
public:
  explicit FirmwareUpdater(UpdateConfig config = {});

  bool flashSingleBoard(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, LogCallback log_callback = {}) const;

  bool enterSingleBoardBootloader(const com::ComInterfaceID& interface, std::size_t board_index, LogCallback log_callback = {}) const;

  bool flashAllBoardsSequential(const com::ComInterfaceID& interface, const std::string& hex_file_path, LogCallback log_callback = {}) const;

  std::optional<std::uint8_t> detectBootloaderNode(const com::ComInterfaceID& interface) const;

private:
  UpdateConfig _config;

  bool flashSingleBoardImpl(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, const std::string& display_node_label, LogCallback log_callback) const;
  std::size_t countAppBoards(const com::ComInterfaceID& interface) const;
  bool enterBootloaderOnBoard(const com::ComInterfaceID& interface, std::size_t board_index, LogCallback log_callback) const;
};

} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
