#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <francor/franklyboot/msg.h>

#include "BootloaderProtocol.hpp"
#include "FlashLayout.hpp"
#include "sensorring/firmware/FirmwareUpdater.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

class BootloaderDevice {
public:
  BootloaderDevice(BootloaderProtocol& protocol, std::uint8_t node_id, std::string display_node_label = {});

  void init();
  void flashHex(const std::string& hex_path, const LogCallback& log_callback);

private:
  BootloaderProtocol& _protocol;
  std::uint8_t _node_id;
  std::string _display_node_label;
  FlashLayout _layout{};

  std::string nodeLabel() const;
  franklyboot::msg::Msg transact(const franklyboot::msg::Msg& request);
  void transactBatch(const std::vector<franklyboot::msg::Msg>& requests, bool expect_echo);
  std::uint32_t readWord(franklyboot::msg::RequestType request_type);
  void writeWord(franklyboot::msg::RequestType request_type, std::uint8_t packet_id, std::uint32_t word, bool expect_echo);
  void exec(franklyboot::msg::RequestType request_type, std::uint32_t argument, bool expect_echo);
  std::uint32_t calculateAppCrc(const FirmwarePages& pages) const;
  void eraseAllApplicationPages(const LogCallback& log_callback);
  void eraseUnusedPages(const FirmwarePages& pages);
};

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
