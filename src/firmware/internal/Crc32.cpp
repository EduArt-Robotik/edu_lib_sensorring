#include "Crc32.hpp"

#include <array>

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

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

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
