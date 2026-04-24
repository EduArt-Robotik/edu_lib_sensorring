#pragma once

#include <cstddef>
#include <cstdint>

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

std::uint32_t crc32IsoHdlc(const std::uint8_t* data, std::size_t size);

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
