#include "sensorring/firmware/FirmwareUpdater.hpp"

#include <algorithm>
#include <francor/franklyboot/msg.h>
#include <optional>
#include <stdexcept>
#include <thread>

#include "firmware/internal/BootloaderActivator.hpp"
#include "firmware/internal/BootloaderDevice.hpp"
#include "firmware/internal/BootloaderProtocol.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/interface/InterfaceParams.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {

namespace {

using RequestType = franklyboot::msg::RequestType;
using ResultType  = franklyboot::msg::ResultType;
using Msg         = franklyboot::msg::Msg;

void logMessage(const LogCallback& log_callback, const std::string& msg) {
  if (log_callback) {
    try{
      log_callback(msg);
    } catch (...) {
      // Ignore exceptions from the callback.
    }
  }
}

bool isResultOk(ResultType result) {
  return result == ResultType::RES_NONE || result == ResultType::RES_OK;
}

std::size_t countBoards(const com::ComInterfaceID& interface) {
  return SensorBus::queryConnectedDevices(interface).size();
}

} // namespace

FirmwareUpdater::FirmwareUpdater(UpdateConfig config)
    : _config(std::move(config)) {
}

bool FirmwareUpdater::flashSingleBoard(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, LogCallback log_callback) const {
  return flashSingleBoardImpl(interface, node_id, hex_file_path, "", log_callback);
}

bool FirmwareUpdater::enterSingleBoardBootloader(const com::ComInterfaceID& interface, std::size_t board_index, LogCallback log_callback) const {
  const auto boards = SensorBus::queryConnectedDevices(interface);
  if (boards.empty()) {
    logMessage(log_callback, "No board found to enter bootloader.");
    return false;
  }

  if (board_index >= boards.size()) {
    logMessage(log_callback, "Requested board index " + std::to_string(board_index) + " is out of range for current board set.");
    return false;
  }

  auto* interface_impl = com::ComManager::getInstance()->getInterface(interface);
  if (!interface_impl) {
    logMessage(log_callback, "Failed to open interface for bootloader activation.");
    return false;
  }

  const std::uint8_t board_address = static_cast<std::uint8_t>(boards[board_index].idx + 1U);
  internal::BootloaderActivator activator(*interface_impl, _config.bootloader_start_ack_timeout);
  if (!activator.enterBootloader(board_address)) {
    logMessage(log_callback, "Failed to switch board " + std::to_string(board_index) + " (address " + std::to_string(board_address) + ") to bootloader mode within " + std::to_string(_config.bootloader_start_ack_timeout.count()) + " ms.");
    return false;
  }

  logMessage(log_callback, "Board " + std::to_string(board_index) + " switched to bootloader mode.");
  return true;
}

bool FirmwareUpdater::flashSingleBoardImpl(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, const std::string& display_node_label, LogCallback log_callback) const {
  try {
    auto* interface_impl = com::ComManager::getInstance()->getInterface(interface);
    if (!interface_impl) {
      logMessage(log_callback, "Failed to open interface for firmware flash.");
      return false;
    }

    internal::BootloaderProtocol protocol(*interface_impl, _config.can_timeout);

    internal::BootloaderDevice device(protocol, node_id, display_node_label);
    device.init();
    device.flashHex(hex_file_path, log_callback);
    return true;
  } catch (const std::exception& e) {
    logMessage(log_callback, std::string("Single-board flash failed: ") + e.what());
    return false;
  }
}

bool FirmwareUpdater::flashAllBoardsSequential(const com::ComInterfaceID& interface, const std::string& hex_file_path, LogCallback log_callback) const {
  std::size_t flashed_boards = 0U;
  unsigned int flashed_nodes     = 0U;

  std::size_t last_known_count = countBoards(interface);

  logMessage(log_callback, "Starting sequential update on " + interface.name + ".");
  logMessage(log_callback, "Initial board count: " + std::to_string(last_known_count));

  auto detectBootloaderWithRetries = [&]() -> std::optional<std::uint8_t> {
    for (unsigned int attempt = 0; attempt < _config.bootloader_detect_retries; ++attempt) {
      const auto node_id = detectBootloaderNode(interface);
      if (node_id.has_value()) {
        return node_id;
      }
      logMessage(log_callback, "Detecting boards already in bootloader mode... attempt " + std::to_string(attempt + 1U) + "/" + std::to_string(_config.bootloader_detect_retries));
      std::this_thread::sleep_for(_config.bootloader_detect_retry_delay);
    }
    return std::nullopt;
  };

  while (true) {
    const std::size_t app_count_now = countBoards(interface);
    if (app_count_now < flashed_boards) {
      logMessage(log_callback, "Board enumeration shrank unexpectedly (have " + std::to_string(app_count_now) + ", already flashed " + std::to_string(flashed_boards) + ").");
      return false;
    }

    while (flashed_boards < app_count_now) {
      const std::size_t board_index        = flashed_boards;
      const std::string display_node_label = "Node " + std::to_string(board_index + 1U) + "/" + std::to_string(app_count_now);
      if (!enterSingleBoardBootloader(interface, board_index, log_callback)) {
        return false;
      }

      const auto node_id = detectBootloaderWithRetries();
      if (!node_id.has_value()) {
        logMessage(log_callback, "Could not detect bootloader node after switching board " + std::to_string(board_index) + ".");
        return false;
      }

      if (!flashSingleBoardImpl(interface, *node_id, hex_file_path, display_node_label, log_callback)) {
        logMessage(log_callback, "Flashing " + display_node_label + " failed (bootloader node " + std::to_string(*node_id) + "). Stopping update.");
        return false;
      }

      ++flashed_nodes;
      ++flashed_boards;
      std::this_thread::sleep_for(_config.settle_delay_after_flash);
      logMessage(log_callback, "Flashed board index " + std::to_string(board_index) + ".");
    }

    const auto boundary_node = detectBootloaderWithRetries();
    if (!boundary_node.has_value()) {
      break;
    }

    const std::string boundary_label = "Node " + std::to_string(app_count_now + 1U) + "/" + std::to_string(app_count_now + 1U);
    if (!flashSingleBoardImpl(interface, *boundary_node, hex_file_path, boundary_label, log_callback)) {
      logMessage(log_callback, "Flashing boundary " + boundary_label + " failed (bootloader node " + std::to_string(*boundary_node) + "). Stopping update.");
      return false;
    }

    ++flashed_nodes;
    std::this_thread::sleep_for(_config.settle_delay_after_flash);

    std::size_t app_count_after_boundary = countBoards(interface);
    for (unsigned int attempt = 0U; app_count_after_boundary <= app_count_now && attempt < _config.bootloader_detect_retries; ++attempt) {
      logMessage(log_callback, "Waiting for boundary node to boot application and advance chain... attempt " + std::to_string(attempt + 1U) + "/" + std::to_string(_config.bootloader_detect_retries));
      std::this_thread::sleep_for(_config.bootloader_detect_retry_delay);
      app_count_after_boundary = countBoards(interface);
    }

    if (app_count_after_boundary <= app_count_now) {
      logMessage(
          log_callback, "Boundary node flashed, but board count did not advance (" + std::to_string(app_count_now) + " -> " + std::to_string(app_count_after_boundary)
                            + "). The flashed image likely stayed in bootloader mode, so downstream boards remain power-isolated.");
      return false;
    }

    // The boundary node we just flashed has now transitioned from
    // bootloader-only to an enumerable board. Treat it as already
    // processed, otherwise the next loop iteration re-targets board index 0
    // again and can push an already-flashed board back into bootloader mode.
    flashed_boards   = app_count_after_boundary;
    last_known_count = app_count_after_boundary;
    logMessage(log_callback, "Sequential update advanced to " + std::to_string(last_known_count) + " board(s).");
  }

  logMessage(log_callback, "Sequential update finished.");
  if (flashed_nodes == 0U) {
    logMessage(log_callback, "No node was flashed. Treating update as failed.");
    return false;
  }
  return true;
}

std::optional<std::uint8_t> FirmwareUpdater::detectBootloaderNode(const com::ComInterfaceID& interface) const {
  try {
    auto* interface_impl = com::ComManager::getInstance()->getInterface(interface);
    if (!interface_impl) {
      return std::nullopt;
    }

    internal::BootloaderProtocol protocol(*interface_impl, _config.can_timeout);

    Msg ping(RequestType::REQ_PING, ResultType::RES_NONE, 0U);
    ping.data = { 0U, 0U, 0U, 0U };
    protocol.sendRequest(ping);

    // The bootloader responds on broadcast, so we cannot recover a physical
    // node id from the wire. Hardware guarantees that at most one board is in
    // bootloader mode at a time, so any successful ping reply means "the one
    // bootloader board is alive". We return a synthetic id (0) to keep the
    // existing optional<bool>-style callers happy without pretending to know
    // which board it actually is.
    while (true) {
      const auto rx = protocol.receive();
      if (!rx.has_value()) {
        return std::nullopt;
      }

      if (rx->request == RequestType::REQ_PING && isResultOk(rx->result)) {
        return std::uint8_t{ 0U };
      }
    }
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<std::uint8_t> FirmwareUpdater::detectBootloaderNodeWithRetries(const com::ComInterfaceID& interface) {
  for (unsigned int attempt = 0; attempt < _config.bootloader_detect_retries; ++attempt) {
    const auto node_id = detectBootloaderNode(interface);
    if (node_id.has_value()) {
      return node_id;
    }
    std::this_thread::sleep_for(_config.bootloader_detect_retry_delay);
  }

  return std::nullopt;
}

} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
