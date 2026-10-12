#include "BootloaderDevice.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

#include "Crc32.hpp"
#include "HexFile.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

namespace {

using RequestType                                 = franklyboot::msg::RequestType;
using ResultType                                  = franklyboot::msg::ResultType;
using Msg                                         = franklyboot::msg::Msg;
constexpr std::size_t MAX_BOOT_COMMANDS_PER_FRAME = 7U;

void logMessage(const LogCallback& log_callback, const std::string& msg) {
  if (log_callback) {
    log_callback(msg);
  }
}

bool isResultOk(ResultType result) {
  return result == ResultType::RES_NONE || result == ResultType::RES_OK;
}

} // namespace

BootloaderDevice::BootloaderDevice(BootloaderProtocol& protocol, std::uint8_t node_id, std::string display_node_label)
    : _protocol(protocol)
    , _node_id(node_id)
    , _display_node_label(std::move(display_node_label)) {
}

void BootloaderDevice::init() {
  _layout.flash_start        = readWord(RequestType::REQ_FLASH_INFO_START_ADDR);
  _layout.flash_page_size    = readWord(RequestType::REQ_FLASH_INFO_PAGE_SIZE);
  _layout.flash_num_pages    = readWord(RequestType::REQ_FLASH_INFO_NUM_PAGES);
  _layout.app_start_page_idx = readWord(RequestType::REQ_APP_INFO_PAGE_IDX);
  _layout.app_start          = _layout.flash_start + _layout.app_start_page_idx * _layout.flash_page_size;
  _layout.app_num_pages      = _layout.flash_num_pages - _layout.app_start_page_idx;
}

void BootloaderDevice::flashHex(const std::string& hex_path, const LogCallback& log_callback) {
  const auto fw_map                 = parseHexFile(hex_path);
  const auto pages                  = buildPagesForApplication(fw_map, _layout);
  const std::array<char, 4> spinner = { '|', '/', '-', '\\' };
  std::size_t page_counter          = 0U;

  eraseAllApplicationPages(log_callback);
  logMessage(log_callback, nodeLabel() + ": flashing " + std::to_string(pages.size()) + " page(s).");

  for (const auto& [page_id, page_bytes] : pages) {
    ++page_counter;
    const char spin = spinner[(page_counter - 1U) % spinner.size()];
    logMessage(log_callback, nodeLabel() + " [" + std::string(1, spin) + "] page " + std::to_string(page_counter) + "/" + std::to_string(pages.size()) + " (flash page " + std::to_string(_layout.app_start_page_idx + page_id) + ")");

    const std::uint32_t flash_page_id = _layout.app_start_page_idx + page_id;

    exec(RequestType::REQ_PAGE_BUFFER_CLEAR, 0U, true);

    for (std::size_t word_idx = 0; word_idx < (page_bytes.size() / 4U);) {
      std::vector<Msg> write_batch;
      write_batch.reserve(MAX_BOOT_COMMANDS_PER_FRAME);
      const std::size_t batch_end = std::min(word_idx + MAX_BOOT_COMMANDS_PER_FRAME, page_bytes.size() / 4U);
      for (; word_idx < batch_end; ++word_idx) {
        const std::size_t byte_idx = word_idx * 4U;
        const std::uint32_t word   = static_cast<std::uint32_t>(page_bytes[byte_idx]) | (static_cast<std::uint32_t>(page_bytes[byte_idx + 1U]) << 8U) | (static_cast<std::uint32_t>(page_bytes[byte_idx + 2U]) << 16U)
                                   | (static_cast<std::uint32_t>(page_bytes[byte_idx + 3U]) << 24U);
        Msg req(RequestType::REQ_PAGE_BUFFER_WRITE_WORD, ResultType::RES_NONE, static_cast<std::uint8_t>(word_idx % 256U));
        franklyboot::msg::convertU32ToMsgData(word, req.data);
        write_batch.push_back(req);
      }
      transactBatch(write_batch, true);
    }

    const std::uint32_t dev_crc  = readWord(RequestType::REQ_PAGE_BUFFER_CALC_CRC);
    const std::uint32_t calc_crc = crc32IsoHdlc(page_bytes.data(), page_bytes.size());
    if (dev_crc != calc_crc) {
      throw std::runtime_error("Node " + std::to_string(_node_id) + ": page CRC mismatch while flashing page " + std::to_string(page_id));
    }

    exec(RequestType::REQ_PAGE_BUFFER_WRITE_TO_FLASH, flash_page_id, true);
  }

  const std::uint32_t app_crc = calculateAppCrc(pages);
  if (readWord(RequestType::REQ_APP_INFO_CRC_CALC) != app_crc) {
    logMessage(log_callback, nodeLabel() + ": CRC mismatch after write, erasing unused pages and retrying.");
    eraseUnusedPages(pages);
    if (readWord(RequestType::REQ_APP_INFO_CRC_CALC) != app_crc) {
      throw std::runtime_error("Node " + std::to_string(_node_id) + ": application CRC check failed after flash");
    }
  }

  exec(RequestType::REQ_FLASH_WRITE_APP_CRC, app_crc, true);

  // Fire and forget request to start the application
  // exec(RequestType::REQ_START_APP, 0U, true);
  _protocol.sendRequestNoWait(Msg(RequestType::REQ_START_APP, ResultType::RES_NONE, 0U));

  logMessage(log_callback, "Flashed " + nodeLabel() + " successfully.");
}

std::string BootloaderDevice::nodeLabel() const {
  if (!_display_node_label.empty()) {
    return _display_node_label;
  }
  return "Node " + std::to_string(_node_id);
}

Msg BootloaderDevice::transact(const Msg& request) {
  _protocol.sendRequest(request);
  const auto rx = _protocol.receive();
  if (!rx.has_value()) {
    throw std::runtime_error("No response from bootloader node " + std::to_string(_node_id));
  }

  const auto& response = *rx;
  if (response.request != request.request || response.packet_id != request.packet_id) {
    throw std::runtime_error("Bootloader response mismatch on node " + std::to_string(_node_id));
  }
  if (!isResultOk(response.result)) {
    throw std::runtime_error("Bootloader request failed with result " + std::to_string(static_cast<int>(response.result)));
  }
  return response;
}

void BootloaderDevice::transactBatch(const std::vector<Msg>& requests, bool expect_echo) {
  if (requests.empty()) {
    return;
  }
  _protocol.sendRequests(requests);
  for (const auto& request : requests) {
    const auto rx = _protocol.receive();
    if (!rx.has_value()) {
      throw std::runtime_error("No response from bootloader node " + std::to_string(_node_id));
    }

    const auto& response = *rx;
    if (response.request != request.request || response.packet_id != request.packet_id) {
      throw std::runtime_error("Bootloader response mismatch on node " + std::to_string(_node_id));
    }
    if (!isResultOk(response.result)) {
      throw std::runtime_error("Bootloader request failed with result " + std::to_string(static_cast<int>(response.result)));
    }
    if (expect_echo && response.data != request.data) {
      throw std::runtime_error("Bootloader echoed invalid response data");
    }
  }
}

std::uint32_t BootloaderDevice::readWord(RequestType request_type) {
  Msg req(request_type, ResultType::RES_NONE, 0U);
  req.data       = { 0U, 0U, 0U, 0U };
  const auto rsp = transact(req);
  return franklyboot::msg::convertMsgDataToU32(rsp.data);
}

void BootloaderDevice::writeWord(RequestType request_type, std::uint8_t packet_id, std::uint32_t word, bool expect_echo) {
  Msg req(request_type, ResultType::RES_NONE, packet_id);
  franklyboot::msg::convertU32ToMsgData(word, req.data);
  const auto rsp = transact(req);
  if (expect_echo && rsp.data != req.data) {
    throw std::runtime_error("Bootloader echoed invalid response data");
  }
}

void BootloaderDevice::exec(RequestType request_type, std::uint32_t argument, bool expect_echo) {
  writeWord(request_type, 0U, argument, expect_echo);
}

std::uint32_t BootloaderDevice::calculateAppCrc(const FirmwarePages& pages) const {
  std::vector<std::uint8_t> app_bytes(_layout.app_num_pages * _layout.flash_page_size, FLASH_DFT_VALUE);
  for (const auto& [page_id, page_data] : pages) {
    const std::size_t offset = static_cast<std::size_t>(page_id) * _layout.flash_page_size;
    std::copy(page_data.begin(), page_data.end(), app_bytes.begin() + static_cast<std::ptrdiff_t>(offset));
  }
  if (app_bytes.size() < 4U) {
    throw std::runtime_error("Application area too small to hold CRC");
  }
  app_bytes.resize(app_bytes.size() - 4U);
  return crc32IsoHdlc(app_bytes.data(), app_bytes.size());
}

void BootloaderDevice::eraseAllApplicationPages(const LogCallback& log_callback) {
  logMessage(log_callback, nodeLabel() + ": erasing full application flash area.");
  std::vector<Msg> erase_batch;
  erase_batch.reserve(MAX_BOOT_COMMANDS_PER_FRAME);
  for (std::uint32_t page_id = 0; page_id < _layout.app_num_pages; ++page_id) {
    Msg req(RequestType::REQ_FLASH_WRITE_ERASE_PAGE, ResultType::RES_NONE, static_cast<std::uint8_t>(page_id % 256U));
    franklyboot::msg::convertU32ToMsgData(_layout.app_start_page_idx + page_id, req.data);
    erase_batch.push_back(req);
    if (erase_batch.size() == MAX_BOOT_COMMANDS_PER_FRAME) {
      transactBatch(erase_batch, true);
      erase_batch.clear();
    }
  }
  if (!erase_batch.empty()) {
    transactBatch(erase_batch, true);
  }
  logMessage(log_callback, nodeLabel() + ": full application flash erase done.");
}

void BootloaderDevice::eraseUnusedPages(const FirmwarePages& pages) {
  std::vector<Msg> erase_batch;
  erase_batch.reserve(MAX_BOOT_COMMANDS_PER_FRAME);
  for (std::uint32_t page_id = 0; page_id < _layout.app_num_pages; ++page_id) {
    if (pages.find(page_id) == pages.end()) {
      Msg req(RequestType::REQ_FLASH_WRITE_ERASE_PAGE, ResultType::RES_NONE, static_cast<std::uint8_t>(page_id % 256U));
      franklyboot::msg::convertU32ToMsgData(_layout.app_start_page_idx + page_id, req.data);
      erase_batch.push_back(req);
      if (erase_batch.size() == MAX_BOOT_COMMANDS_PER_FRAME) {
        transactBatch(erase_batch, true);
        erase_batch.clear();
      }
    }
  }
  if (!erase_batch.empty()) {
    transactBatch(erase_batch, true);
  }
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
