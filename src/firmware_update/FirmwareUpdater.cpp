#include "sensorring/firmware_update/FirmwareUpdater.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <thread>

#include <francor/franklyboot/msg.h>

#include "firmware_update/internal/BootloaderDevice.hpp"
#include "firmware_update/internal/BootloaderProtocol.hpp"
#include "firmware_update/internal/BootloaderActivator.hpp"
#include "interface/ComManager.hpp"
#include "interface/can/CanInterface.hpp"
#include "sensorring/SensorRingFactory.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {

namespace {

using RequestType = franklyboot::msg::RequestType;
using ResultType  = franklyboot::msg::ResultType;
using Msg         = franklyboot::msg::Msg;

void logMessage(const LogCallback& log_callback, const std::string& msg) {
  if (log_callback) {
    log_callback(msg);
  }
}

bool isResultOk(ResultType result) {
  return result == ResultType::RES_NONE || result == ResultType::RES_OK;
}

com::CanInterface* getCanInterface(const com::ComInterfaceID& interface) {
  auto* interface_impl = com::ComManager::getInstance()->getInterface(interface);
  return dynamic_cast<com::CanInterface*>(interface_impl);
}

std::vector<device::EnumerationInformation> enumerateBoardsOnInterface(const com::ComInterfaceID& interface) {
  ring::SensorRingFactory factory(ring::ValidationMode::Relaxed);
  factory.addInterface(interface);
  const auto enumeration = factory.enumerate();

  auto it = enumeration.find(interface);
  if (it == enumeration.end() && enumeration.size() == 1U) {
    it = enumeration.begin();
  }
  if (it == enumeration.end()) {
    return {};
  }

  auto boards = it->second;
  std::sort(boards.begin(), boards.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.idx < rhs.idx;
  });
  return boards;
}

} // namespace

FirmwareUpdater::FirmwareUpdater(UpdateConfig config)
    : _config(std::move(config)) {
}

bool FirmwareUpdater::flashSingleBoard(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, LogCallback log_callback) const {
  return flashSingleBoardImpl(interface, node_id, hex_file_path, "", log_callback);
}

bool FirmwareUpdater::enterSingleBoardBootloader(const com::ComInterfaceID& interface, std::size_t board_index, LogCallback log_callback) const {
  return enterBootloaderOnBoard(interface, board_index, log_callback);
}

bool FirmwareUpdater::flashSingleBoardImpl(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, const std::string& display_node_label, LogCallback log_callback) const {
  try {
    auto* can_interface = getCanInterface(interface);
    if (!can_interface) {
      logMessage(log_callback, "Requested interface does not provide CAN frame access.");
      return false;
    }

    internal::BootloaderProtocol protocol(*can_interface, _config.can_timeout);
    protocol.setSpecificNodeMode(node_id);

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
  std::size_t flashed_app_boards = 0U;
  unsigned int flashed_nodes     = 0U;

  std::size_t last_known_app_count = countAppBoards(interface);

  logMessage(log_callback, "Starting sequential update on " + interface.name + ".");
  logMessage(log_callback, "Initial app board count: " + std::to_string(last_known_app_count));

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
    const std::size_t app_count_now = countAppBoards(interface);
    if (app_count_now < flashed_app_boards) {
      logMessage(log_callback, "Board enumeration shrank unexpectedly (have " + std::to_string(app_count_now) + ", already flashed " + std::to_string(flashed_app_boards) + ").");
      return false;
    }

    while (flashed_app_boards < app_count_now) {
      const std::size_t board_index        = flashed_app_boards;
      const std::string display_node_label = "Node " + std::to_string(board_index + 1U) + "/" + std::to_string(app_count_now);
      if (!enterBootloaderOnBoard(interface, board_index, log_callback)) {
        return false;
      }

      const auto node_id = detectBootloaderWithRetries();
      if (!node_id.has_value()) {
        logMessage(log_callback, "Could not detect bootloader node after switching app board " + std::to_string(board_index) + ".");
        return false;
      }

      if (!flashSingleBoardImpl(interface, *node_id, hex_file_path, display_node_label, log_callback)) {
        logMessage(log_callback, "Flashing " + display_node_label + " failed (bootloader node " + std::to_string(*node_id) + "). Stopping update.");
        return false;
      }

      ++flashed_nodes;
      ++flashed_app_boards;
      std::this_thread::sleep_for(_config.settle_delay_after_flash);
      logMessage(log_callback, "Flashed app board index " + std::to_string(board_index) + ".");
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

    const std::size_t app_count_after_boundary = countAppBoards(interface);
    if (app_count_after_boundary <= app_count_now) {
      logMessage(log_callback, "Boundary node flashed, but app board count did not advance (" + std::to_string(app_count_now) + " -> " + std::to_string(app_count_after_boundary) + ").");
      return false;
    }

    last_known_app_count = app_count_after_boundary;
    logMessage(log_callback, "Sequential update advanced to " + std::to_string(last_known_app_count) + " app board(s).");
  }

  logMessage(log_callback, "Sequential update finished.");
  if (flashed_nodes == 0U) {
    logMessage(log_callback, "No node was flashed. Treating update as failed.");
    return false;
  }
  return true;
}

std::size_t FirmwareUpdater::countAppBoards(const com::ComInterfaceID& interface) const {
  return enumerateBoardsOnInterface(interface).size();
}

bool FirmwareUpdater::enterBootloaderOnBoard(const com::ComInterfaceID& interface, const std::size_t board_index, LogCallback log_callback) const {
  const auto boards = enumerateBoardsOnInterface(interface);
  if (boards.empty()) {
    logMessage(log_callback, "No app board found to enter bootloader.");
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
  internal::BootloaderActivator activator(*interface_impl, _config.can_timeout);
  if (!activator.enterBootloader(board_address)) {
    logMessage(log_callback, "Failed to switch board " + std::to_string(board_index) + " to bootloader mode.");
    return false;
  }

  logMessage(log_callback, "Board " + std::to_string(board_index) + " switched to bootloader mode.");
  return true;
}

std::optional<std::uint8_t> FirmwareUpdater::detectBootloaderNode(const com::ComInterfaceID& interface) const {
  try {
    auto* can_interface = getCanInterface(interface);
    if (!can_interface) {
      return std::nullopt;
    }

    internal::BootloaderProtocol protocol(*can_interface, _config.can_timeout);
    protocol.setBroadcastMode();

    Msg ping(RequestType::REQ_PING, ResultType::RES_NONE, 0U);
    ping.data = { 0U, 0U, 0U, 0U };
    protocol.sendRequest(ping);

    std::optional<std::uint8_t> detected_node;
    while (true) {
      const auto rx = protocol.receive();
      if (!rx.has_value()) {
        break;
      }

      if (rx->msg.request == RequestType::REQ_PING && isResultOk(rx->msg.result)) {
        detected_node = internal::BootloaderProtocol::nodeFromCanId(rx->can_id);
        break;
      }
    }
    return detected_node;
  } catch (...) {
    return std::nullopt;
  }
}

} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
