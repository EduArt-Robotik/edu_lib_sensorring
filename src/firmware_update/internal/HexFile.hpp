#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace eduart {
namespace sensorring {
namespace firmware_update {
namespace internal {

using FirmwareByteMap = std::map<std::uint32_t, std::uint8_t>;

FirmwareByteMap parseHexFile(const std::string& file_path);

} // namespace internal
} // namespace firmware_update
} // namespace sensorring
} // namespace eduart
