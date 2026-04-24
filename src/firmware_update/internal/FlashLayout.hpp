#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>

#include "HexFile.hpp"

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

constexpr std::uint8_t FLASH_DFT_VALUE = 0xFFU;

struct FlashLayout {
  std::uint32_t flash_start{};
  std::uint32_t flash_page_size{};
  std::uint32_t flash_num_pages{};
  std::uint32_t app_start_page_idx{};
  std::uint32_t app_start{};
  std::uint32_t app_num_pages{};
};

using FirmwarePages = std::map<std::uint32_t, std::vector<std::uint8_t> >;

inline FirmwarePages buildPagesForApplication(const FirmwareByteMap& map, const FlashLayout& layout) {
  FirmwarePages pages;
  const std::uint32_t app_end = layout.app_start + layout.app_num_pages * layout.flash_page_size;

  for (const auto& [address, value] : map) {
    if (address < layout.app_start || address >= app_end) {
      throw std::runtime_error("HEX address out of application flash range");
    }
    const std::uint32_t page_id = (address - layout.app_start) / layout.flash_page_size;
    const std::uint32_t idx     = (address - layout.app_start) % layout.flash_page_size;
    auto it                     = pages.find(page_id);
    if (it == pages.end()) {
      it = pages.emplace(page_id, std::vector<std::uint8_t>(layout.flash_page_size, FLASH_DFT_VALUE)).first;
    }
    it->second[idx] = value;
  }
  return pages;
}

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
