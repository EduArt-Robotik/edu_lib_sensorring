#include "HexFile.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

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

    if (line.empty() || line.front() != ':') {
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

    if (record_type == 0x04U) {
      if (data.size() != 2U) {
        throw std::runtime_error("Invalid HEX ELA record");
      }
      extended_address = (static_cast<std::uint32_t>(data[0]) << 24U) | (static_cast<std::uint32_t>(data[1]) << 16U);
    } else if (record_type == 0x00U) {
      const std::uint32_t address_base = extended_address | offset;
      for (std::uint32_t i = 0; i < data.size(); ++i) {
        fw[address_base + i] = data[i];
      }
    } else if (record_type == 0x01U) {
      break;
    }
  }

  if (fw.empty()) {
    throw std::runtime_error("HEX file does not contain valid data");
  }
  return fw;
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
