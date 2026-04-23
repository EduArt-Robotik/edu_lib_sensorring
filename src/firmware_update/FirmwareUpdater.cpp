#include "sensorring/firmware_update/FirmwareUpdater.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <francor/franklyboot/msg.h>
#include <fstream>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <map>
#include <net/if.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "sensorring/SensorRingFactory.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {

namespace {

using RequestType = franklyboot::msg::RequestType;
using ResultType  = franklyboot::msg::ResultType;
using Msg         = franklyboot::msg::Msg;

constexpr std::uint32_t CAN_BASE_ID      = 0x781U;
constexpr std::uint32_t CAN_BROADCAST_ID = 0x780U;
constexpr std::uint32_t CAN_MAX_ID       = 0x7FFU;

constexpr std::uint8_t FLASH_DFT_VALUE = 0xFFU;

void logMessage(const LogCallback& log_callback, const std::string& msg) {
  if (log_callback) {
    log_callback(msg);
  }
}

bool isResultOk(ResultType result) {
  return result == ResultType::RES_NONE || result == ResultType::RES_OK;
}

std::uint8_t nodeFromCanId(std::uint32_t id) {
  if (id < CAN_BASE_ID) {
    throw std::runtime_error("Invalid CAN response ID " + std::to_string(id));
  }
  return static_cast<std::uint8_t>((id - CAN_BASE_ID) / 2U);
}

std::uint32_t responseCanIdForNode(std::uint8_t node_id) {
  return CAN_BASE_ID + static_cast<std::uint32_t>(node_id) * 2U + 1U;
}

std::uint32_t crc32IsoHdlc(const std::uint8_t* data, std::size_t size) {
  static std::array<std::uint32_t, 256> table = [] {
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t i = 0; i < 256; ++i) {
      std::uint32_t c = i;
      for (std::uint32_t j = 0; j < 8; ++j) {
        c = (c & 1U) ? (0xEDB88320U ^ (c >> 1U)) : (c >> 1U);
      }
      t[i] = c;
    }
    return t;
  }();

  std::uint32_t crc = 0xFFFFFFFFU;
  for (std::size_t i = 0; i < size; ++i) {
    const std::uint8_t idx = static_cast<std::uint8_t>((crc ^ data[i]) & 0xFFU);
    crc                    = table[idx] ^ (crc >> 8U);
  }
  return crc ^ 0xFFFFFFFFU;
}

class SocketCanBootCom {
public:
  struct RxFrame {
    std::uint32_t can_id{};
    Msg msg{};
  };

  explicit SocketCanBootCom(std::chrono::milliseconds timeout)
      : _timeout(timeout) {}

  ~SocketCanBootCom() { closeSocket(); }

  void open(const std::string& if_name) {
    closeSocket();

    _socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (_socket < 0) {
      throw std::runtime_error("Failed to open CAN socket: " + std::string(std::strerror(errno)));
    }

    int canfd_enable = 1;
    if (setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_enable, sizeof(canfd_enable)) < 0) {
      throw std::runtime_error("Failed to enable CAN FD mode on socket");
    }

    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, if_name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(_socket, SIOCGIFFLAGS, &ifr) < 0) {
      throw std::runtime_error("Unable to get interface flags for " + if_name);
    }
    if (!(ifr.ifr_flags & IFF_UP)) {
      throw std::runtime_error("CAN interface \"" + if_name + "\" is DOWN");
    }
    if (!(ifr.ifr_flags & IFF_RUNNING)) {
      throw std::runtime_error("CAN interface \"" + if_name + "\" is UP but not RUNNING");
    }

    if (ioctl(_socket, SIOCGIFINDEX, &ifr) < 0) {
      throw std::runtime_error("Unable to get interface index for " + if_name);
    }

    struct sockaddr_can addr{};
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
      throw std::runtime_error("Unable to bind CAN socket: " + std::string(std::strerror(errno)));
    }

    clearRxQueue();
  }

  void setBroadcastMode() {
    struct can_filter filter{};
    // Restrict incoming frames to bootloader ID region (0x780..0x7FF)
    filter.can_id   = CAN_BROADCAST_ID;
    filter.can_mask = CAN_BROADCAST_ID;
    if (setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) < 0) {
      throw std::runtime_error("Failed to configure CAN broadcast filter");
    }
  }

  void setSpecificNodeMode(std::uint8_t node_id) {
    const std::uint32_t rx_id = responseCanIdForNode(node_id);
    struct can_filter filter{};
    filter.can_id   = rx_id;
    filter.can_mask = CAN_MAX_ID;
    if (setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) < 0) {
      throw std::runtime_error("Failed to configure CAN node filter");
    }
  }

  void sendRequest(const Msg& msg) {
    struct can_frame frame{};
    frame.can_id  = CAN_BROADCAST_ID;
    frame.can_dlc = 8U;

    const auto raw = franklyboot::msg::convertMsgToBytes(msg);
    for (std::size_t i = 0; i < raw.size(); ++i) {
      frame.data[i] = raw[i];
    }

    const auto written = write(_socket, &frame, sizeof(frame));
    if (written != static_cast<ssize_t>(sizeof(frame))) {
      throw std::runtime_error("Failed to send CAN frame");
    }
  }

  std::optional<RxFrame> receive() const {
    fd_set set;
    const auto deadline = std::chrono::steady_clock::now() + _timeout;

    while (std::chrono::steady_clock::now() < deadline) {
      FD_ZERO(&set);
      FD_SET(_socket, &set);

      const auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(deadline - std::chrono::steady_clock::now());
      if (remaining.count() <= 0) {
        break;
      }

      timeval tv{};
      tv.tv_sec  = static_cast<time_t>(remaining.count() / 1000000);
      tv.tv_usec = static_cast<suseconds_t>(remaining.count() % 1000000);

      const int ready = select(_socket + 1, &set, nullptr, nullptr, &tv);
      if (ready <= 0) {
        continue;
      }

      struct canfd_frame frame{};
      const auto n = read(_socket, &frame, sizeof(frame));
      if (n != CAN_MTU && n != CANFD_MTU) {
        continue;
      }
      if (frame.len < 8U) {
        continue;
      }

      franklyboot::msg::MsgRaw raw{};
      for (std::size_t i = 0; i < raw.size(); ++i) {
        raw[i] = frame.data[i];
      }

      RxFrame out;
      out.can_id = (frame.can_id & CAN_SFF_MASK);
      out.msg    = franklyboot::msg::convertBytesToMsg(raw);
      return out;
    }

    return std::nullopt;
  }

private:
  int _socket{ -1 };
  std::chrono::milliseconds _timeout;

  void closeSocket() {
    if (_socket >= 0) {
      close(_socket);
      _socket = -1;
    }
  }

  void clearRxQueue() const {
    if (_socket < 0) {
      return;
    }

    const int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
    struct can_frame frame{};
    while (read(_socket, &frame, sizeof(frame)) > 0) {
    }
    fcntl(_socket, F_SETFL, flags);
  }
};

struct FlashLayout {
  std::uint32_t flash_start{};
  std::uint32_t flash_page_size{};
  std::uint32_t flash_num_pages{};
  std::uint32_t app_start_page_idx{};
  std::uint32_t app_start{};
  std::uint32_t app_num_pages{};
};

using FirmwareByteMap = std::map<std::uint32_t, std::uint8_t>;

FirmwareByteMap parseHexFile(const std::string& file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open HEX file: " + file_path);
  }

  auto hexByte = [](const std::string& s, std::size_t pos) -> std::uint8_t {
    return static_cast<std::uint8_t>(std::stoul(s.substr(pos, 2), nullptr, 16));
  };
  auto hexWord = [](const std::string& s, std::size_t pos) -> std::uint16_t {
    return static_cast<std::uint16_t>(std::stoul(s.substr(pos, 4), nullptr, 16));
  };

  FirmwareByteMap fw;
  std::string line;
  std::uint32_t extended_address = 0U;

  while (std::getline(file, line)) {
    while (!line.empty() && (line.back() == '\r' || std::isspace(static_cast<unsigned char>(line.back())))) {
      line.pop_back();
    }

    if (line.empty()) {
      continue;
    }
    if (line.front() != ':') {
      continue;
    }
    if (line.size() < 11) {
      throw std::runtime_error("Invalid HEX line: too short");
    }

    const std::uint8_t byte_count   = hexByte(line, 1);
    const std::uint16_t offset      = hexWord(line, 3);
    const std::uint8_t record_type  = hexByte(line, 7);
    const std::size_t expected_size = 1 + 2 + 4 + 2 + static_cast<std::size_t>(byte_count) * 2 + 2;
    if (line.size() != expected_size) {
      throw std::runtime_error("Invalid HEX line: byte count mismatch");
    }

    std::uint32_t checksum = byte_count + static_cast<std::uint8_t>(offset >> 8U) + static_cast<std::uint8_t>(offset & 0xFFU) + record_type;
    std::vector<std::uint8_t> data;
    data.reserve(byte_count);
    for (std::uint8_t i = 0; i < byte_count; ++i) {
      const auto v = hexByte(line, 9 + static_cast<std::size_t>(i) * 2U);
      data.push_back(v);
      checksum += v;
    }
    checksum                     = ((~checksum + 1U) & 0xFFU);
    const auto checksum_expected = hexByte(line, 9 + static_cast<std::size_t>(byte_count) * 2U);
    if (checksum != checksum_expected) {
      throw std::runtime_error("Invalid HEX line: checksum mismatch");
    }

    if (record_type == 0x04U) { // extended linear address
      if (data.size() != 2U) {
        throw std::runtime_error("Invalid HEX ELA record");
      }
      extended_address = (static_cast<std::uint32_t>(data[0]) << 24U) | (static_cast<std::uint32_t>(data[1]) << 16U);
    } else if (record_type == 0x00U) { // data
      const std::uint32_t address_base = extended_address | offset;
      for (std::uint32_t i = 0; i < data.size(); ++i) {
        fw[address_base + i] = data[i];
      }
    } else if (record_type == 0x01U) { // EOF
      break;
    }
  }

  if (fw.empty()) {
    throw std::runtime_error("HEX file does not contain valid data");
  }
  return fw;
}

class BootloaderDevice {
public:
  BootloaderDevice(SocketCanBootCom& com, std::uint8_t node_id, std::chrono::milliseconds rx_timeout, std::string display_node_label = {})
      : _com(com)
      , _node_id(node_id)
      , _rx_timeout(rx_timeout)
      , _display_node_label(std::move(display_node_label)) {}

  void init() {
    _layout.flash_start        = readWord(RequestType::REQ_FLASH_INFO_START_ADDR);
    _layout.flash_page_size    = readWord(RequestType::REQ_FLASH_INFO_PAGE_SIZE);
    _layout.flash_num_pages    = readWord(RequestType::REQ_FLASH_INFO_NUM_PAGES);
    _layout.app_start_page_idx = readWord(RequestType::REQ_APP_INFO_PAGE_IDX);
    _layout.app_start          = _layout.flash_start + _layout.app_start_page_idx * _layout.flash_page_size;
    _layout.app_num_pages      = _layout.flash_num_pages - _layout.app_start_page_idx;
  }

  void flashHex(const std::string& hex_path, const LogCallback& log_callback) {
    const auto fw_map                 = parseHexFile(hex_path);
    const auto pages                  = buildPages(fw_map);
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

      for (std::size_t word_idx = 0; word_idx < (page_bytes.size() / 4U); ++word_idx) {
        const std::size_t byte_idx = word_idx * 4U;
        const std::uint32_t word   = static_cast<std::uint32_t>(page_bytes[byte_idx]) | (static_cast<std::uint32_t>(page_bytes[byte_idx + 1U]) << 8U) | (static_cast<std::uint32_t>(page_bytes[byte_idx + 2U]) << 16U)
                                   | (static_cast<std::uint32_t>(page_bytes[byte_idx + 3U]) << 24U);

        writeWord(RequestType::REQ_PAGE_BUFFER_WRITE_WORD, static_cast<std::uint8_t>(word_idx % 256U), word, true);
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
    exec(RequestType::REQ_START_APP, 0U, true);
    logMessage(log_callback, "Flashed " + nodeLabel() + " successfully.");
  }

private:
  SocketCanBootCom& _com;
  std::uint8_t _node_id;
  std::chrono::milliseconds _rx_timeout;
  std::string _display_node_label;
  FlashLayout _layout{};

  std::string nodeLabel() const {
    if (!_display_node_label.empty()) {
      return _display_node_label;
    }
    return "Node " + std::to_string(_node_id);
  }

  Msg transact(const Msg& request) {
    _com.sendRequest(request);
    const auto rx = _com.receive();
    if (!rx.has_value()) {
      throw std::runtime_error("No response from bootloader node " + std::to_string(_node_id));
    }

    const auto& response = rx->msg;
    if (response.request != request.request || response.packet_id != request.packet_id) {
      throw std::runtime_error("Bootloader response mismatch on node " + std::to_string(_node_id));
    }
    if (!isResultOk(response.result)) {
      throw std::runtime_error("Bootloader request failed with result " + std::to_string(static_cast<int>(response.result)));
    }
    return response;
  }

  std::uint32_t readWord(RequestType request_type) {
    Msg req(request_type, ResultType::RES_NONE, 0U);
    req.data       = { 0U, 0U, 0U, 0U };
    const auto rsp = transact(req);
    return franklyboot::msg::convertMsgDataToU32(rsp.data);
  }

  void writeWord(RequestType request_type, std::uint8_t packet_id, std::uint32_t word, bool expect_echo) {
    Msg req(request_type, ResultType::RES_NONE, packet_id);
    franklyboot::msg::convertU32ToMsgData(word, req.data);
    const auto rsp = transact(req);
    if (expect_echo && rsp.data != req.data) {
      throw std::runtime_error("Bootloader echoed invalid response data");
    }
  }

  void exec(RequestType request_type, std::uint32_t argument, bool expect_echo) { writeWord(request_type, 0U, argument, expect_echo); }

  std::map<std::uint32_t, std::vector<std::uint8_t> > buildPages(const FirmwareByteMap& map) const {
    std::map<std::uint32_t, std::vector<std::uint8_t> > pages;
    const std::uint32_t app_end = _layout.app_start + _layout.app_num_pages * _layout.flash_page_size;

    for (const auto& [address, value] : map) {
      if (address < _layout.app_start || address >= app_end) {
        throw std::runtime_error("HEX address out of application flash range");
      }
      const std::uint32_t page_id = (address - _layout.app_start) / _layout.flash_page_size;
      const std::uint32_t idx     = (address - _layout.app_start) % _layout.flash_page_size;
      auto it                     = pages.find(page_id);
      if (it == pages.end()) {
        it = pages.emplace(page_id, std::vector<std::uint8_t>(_layout.flash_page_size, FLASH_DFT_VALUE)).first;
      }
      it->second[idx] = value;
    }
    return pages;
  }

  std::uint32_t calculateAppCrc(const std::map<std::uint32_t, std::vector<std::uint8_t> >& pages) const {
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

  void eraseAllApplicationPages(const LogCallback& log_callback) {
    logMessage(log_callback, nodeLabel() + ": erasing full application flash area.");
    for (std::uint32_t page_id = 0; page_id < _layout.app_num_pages; ++page_id) {
      exec(RequestType::REQ_FLASH_WRITE_ERASE_PAGE, _layout.app_start_page_idx + page_id, true);
    }
    logMessage(log_callback, nodeLabel() + ": full application flash erase done.");
  }

  void eraseUnusedPages(const std::map<std::uint32_t, std::vector<std::uint8_t> >& pages) {
    for (std::uint32_t page_id = 0; page_id < _layout.app_num_pages; ++page_id) {
      if (pages.find(page_id) == pages.end()) {
        exec(RequestType::REQ_FLASH_WRITE_ERASE_PAGE, _layout.app_start_page_idx + page_id, true);
      }
    }
  }
};

} // namespace

FirmwareUpdater::FirmwareUpdater(UpdateConfig config)
    : _config(std::move(config)) {
}

bool FirmwareUpdater::flashSingleBoard(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, LogCallback log_callback) const {
  return flashSingleBoardImpl(interface, node_id, hex_file_path, "", log_callback);
}

bool FirmwareUpdater::flashSingleBoardImpl(const com::ComInterfaceID& interface, std::uint8_t node_id, const std::string& hex_file_path, const std::string& display_node_label, LogCallback log_callback) const {
  if (interface.type != com::InterfaceType::SocketCan) {
    logMessage(log_callback, "Firmware update currently supports only SocketCAN.");
    return false;
  }

  try {
    SocketCanBootCom com(_config.can_timeout);
    com.open(interface.name);
    com.setSpecificNodeMode(node_id);

    BootloaderDevice device(com, node_id, _config.can_timeout, display_node_label);
    device.init();
    device.flashHex(hex_file_path, log_callback);
    return true;
  } catch (const std::exception& e) {
    logMessage(log_callback, std::string("Single-board flash failed: ") + e.what());
    return false;
  }
}

bool FirmwareUpdater::flashAllBoardsSequential(const com::ComInterfaceID& interface, const std::string& hex_file_path, LogCallback log_callback) const {
  if (interface.type != com::InterfaceType::SocketCan) {
    logMessage(log_callback, "Sequential ring update currently supports only SocketCAN.");
    return false;
  }

  std::size_t flashed_app_boards   = 0U;
  std::size_t last_known_app_count = countAppBoards(interface);
  unsigned int flashed_nodes       = 0U;

  logMessage(log_callback, "Starting sequential ring update on " + interface.name + ".");
  logMessage(log_callback, "Initial app board count: " + std::to_string(last_known_app_count));

  auto detectBootloaderWithRetries = [&]() -> std::optional<std::uint8_t> {
    for (unsigned int attempt = 0; attempt < _config.bootloader_detect_retries; ++attempt) {
      const auto node_id = detectBootloaderNode(interface);
      if (node_id.has_value()) {
        return node_id;
      }
      logMessage(log_callback, "Detecting boards in bootloader mode... attempt " + std::to_string(attempt + 1U) + "/" + std::to_string(_config.bootloader_detect_retries));
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

    // Step 2: flash all currently detected app boards from first to last.
    while (flashed_app_boards < app_count_now) {
      const std::size_t board_index        = flashed_app_boards;
      const std::string display_node_label = "Node " + std::to_string(board_index + 1U) + "/" + std::to_string(app_count_now);
      if (!enterBootloaderOnBoardIndex(interface, board_index, log_callback)) {
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

    // Step 3: check whether a boundary bootloader board is already active.
    const auto boundary_node = detectBootloaderWithRetries();
    if (!boundary_node.has_value()) {
      // No bootloader board currently breaking the chain.
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
    logMessage(log_callback, "Sequential ring update advanced to " + std::to_string(last_known_app_count) + " app board(s).");
  }

  logMessage(log_callback, "Sequential ring update finished.");
  if (flashed_nodes == 0U) {
    logMessage(log_callback, "No node was flashed. Treating update as failed.");
    return false;
  }
  return true;
}

std::size_t FirmwareUpdater::countAppBoards(const com::ComInterfaceID& interface) const {
  ring::SensorRingFactory factory(ring::ValidationMode::Relaxed);
  factory.addInterface(interface);
  const auto enumeration = factory.enumerate();

  std::size_t count = 0U;
  for (const auto& [iface, boards] : enumeration) {
    (void)iface;
    count += boards.size();
  }
  return count;
}

bool FirmwareUpdater::enterBootloaderOnBoardIndex(const com::ComInterfaceID& interface, const std::size_t board_index, LogCallback log_callback) const {
  ring::SensorRingFactory factory(ring::ValidationMode::Relaxed);
  factory.addInterface(interface);

  const auto enumeration  = factory.enumerate();
  std::size_t board_count = 0U;
  for (const auto& [iface, boards] : enumeration) {
    (void)iface;
    board_count += boards.size();
  }

  if (board_count == 0U) {
    logMessage(log_callback, "No app board found to enter bootloader.");
    return false;
  }

  auto ring = factory.build();
  if (!ring) {
    logMessage(log_callback, "Failed to build SensorRing for bootloader entry.");
    return false;
  }

  const auto buses = ring->getSensorBuses();
  if (buses.empty() || buses.front()->getSensorBoards().empty()) {
    logMessage(log_callback, "No usable board found after SensorRing build.");
    return false;
  }

  const auto boards = buses.front()->getSensorBoards();
  if (board_index >= boards.size()) {
    logMessage(log_callback, "Requested board index " + std::to_string(board_index) + " is out of range for current board set.");
    return false;
  }

  if (!boards[board_index]->enterBootloader()) {
    logMessage(log_callback, "Failed to switch sequential ring board " + std::to_string(board_index) + " to bootloader mode.");
    return false;
  }

  logMessage(log_callback, "Sequential ring board " + std::to_string(board_index) + " switched to bootloader mode.");
  return true;
}

std::optional<std::uint8_t> FirmwareUpdater::detectBootloaderNode(const com::ComInterfaceID& interface) const {
  try {
    SocketCanBootCom com(_config.can_timeout);
    com.open(interface.name);
    com.setBroadcastMode();

    Msg ping(RequestType::REQ_PING, ResultType::RES_NONE, 0U);
    ping.data = { 0U, 0U, 0U, 0U };
    com.sendRequest(ping);

    std::optional<std::uint8_t> detected_node;
    while (true) {
      const auto rx = com.receive();
      if (!rx.has_value()) {
        break;
      }
      const bool id_in_bootloader_range = (rx->can_id >= CAN_BASE_ID) && (rx->can_id <= CAN_MAX_ID);
      if (id_in_bootloader_range && rx->msg.request == RequestType::REQ_PING && isResultOk(rx->msg.result)) {
        detected_node = nodeFromCanId(rx->can_id);
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
