// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   FirmwareUpdater.hpp
 * @author EduArt Robotik GmbH
 * @brief  Utility class for flashing sensor board firmware over CAN.
 * @date   2026-05-08
 */

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

/**
 * @struct UpdateConfig
 * @brief  Timing and retry parameters that control the firmware-update procedure.
 */
struct UpdateConfig {
  std::chrono::milliseconds can_timeout{ 500 };                         ///< Maximum time to wait for a CAN response before declaring a timeout.
  std::chrono::milliseconds bootloader_start_ack_timeout{ 1200 };       ///< Maximum time to wait for the bootloader-start acknowledgement after a reset.
  std::chrono::milliseconds settle_delay_after_flash{ 500 };            ///< Delay inserted after the last flash packet to let the board settle before verification.
  std::chrono::milliseconds bootloader_detect_retry_delay{ 200 };       ///< Pause between consecutive bootloader-detection attempts.
  unsigned int bootloader_detect_retries{ 5 };                          ///< Number of bootloader-detection attempts before giving up.
  unsigned int no_progress_cycles_before_done{ 3 };                     ///< Number of consecutive cycles without progress that signal a completed flash operation.
};

/// @brief Callback invoked by the updater to emit human-readable log messages.
using LogCallback = std::function<void(const std::string&)>;

/**
 * @class FirmwareUpdater
 * @brief Flashes sensor-board firmware images over a CAN interface.
 *
 * FirmwareUpdater uses the frankly bootloader protocol to transfer Intel HEX
 * firmware images to one or more sensor boards connected on a CAN bus. All
 * operations are synchronous and blocking; call them from a dedicated thread
 * if you need to keep the rest of the application responsive.
 */
class FirmwareUpdater {
public:
  /**
   * @brief Constructs a FirmwareUpdater with the given timing/retry configuration.
   * @param config Timing and retry parameters. Defaults to @ref UpdateConfig{}.
   */
  explicit FirmwareUpdater(UpdateConfig config = {});

  /**
   * @brief Flashes a single board identified by its CAN node ID.
   * @param interface     Communication interface to use for the update.
   * @param node_id       CAN node ID of the target board.
   * @param hex_file_path Path to the Intel HEX file that contains the firmware image.
   * @param log_callback  Optional callback that receives progress and status messages.
   * @return @c true if the board was flashed successfully, @c false otherwise.
   */
  bool flashSingleBoard(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, LogCallback log_callback = {}) const;

  /**
   * @brief Requests a single board to enter bootloader mode.
   * @param interface    Communication interface to use.
   * @param board_index  Zero-based index of the board on the ring that should enter bootloader mode.
   * @param log_callback Optional callback that receives status messages.
   * @return @c true if the board acknowledged the bootloader-enter request, @c false otherwise.
   */
  bool enterSingleBoardBootloader(const com::ComInterfaceID& interface, std::size_t board_index, LogCallback log_callback = {}) const;

  /**
   * @brief Flashes every board on the ring sequentially with the same firmware image.
   * @param interface     Communication interface to use.
   * @param hex_file_path Path to the Intel HEX file that contains the firmware image.
   * @param log_callback  Optional callback that receives per-board progress messages.
   * @return @c true if every board was flashed successfully, @c false if any board failed.
   */
  bool flashAllBoardsSequential(const com::ComInterfaceID& interface, const std::string& hex_file_path, LogCallback log_callback = {}) const;

  /**
   * @brief Detects whether any board on the ring is currently in bootloader mode.
   * @param interface Communication interface to scan.
   * @return The CAN node ID of the board in bootloader mode, or @c std::nullopt if none was found.
   */
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
