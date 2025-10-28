#include "EnumerationInformation.hpp"

#include <iomanip>
#include <sstream>

namespace eduart {

std::string Version::toString() const {
  std::ostringstream oss;
  oss << major << '.' << minor << '.' << patch;
  return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Version& v) noexcept {
  return os << v.toString();
}

bool operator==(const Version& lhs, const Version& rhs) noexcept {
  return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
}

inline CommitHash CommitHash::fromBits(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept {
  return CommitHash{ (static_cast<std::uint32_t>(a) << 24) | (static_cast<std::uint32_t>(b) << 16) | (static_cast<std::uint32_t>(c) << 8) | (static_cast<std::uint32_t>(d)) };
}

std::string CommitHash::toString() const {
  std::ostringstream oss;
  oss << std::hex << std::setfill('0') << std::setw(8) << static_cast<int>(hash);
  return oss.str();
}

std::ostream& operator<<(std::ostream& os, const CommitHash& ch) noexcept {
  return os << ch.toString();
}

bool operator==(const CommitHash& lhs, const CommitHash& rhs) noexcept {
  return lhs.hash == rhs.hash;
}

namespace sensor {

std::string to_string(EnumerationState state) {
  switch (state) {
  case EnumerationState::Undefined:
    return "Undefined";
  case EnumerationState::ConfiguredAndConnected:
    return "Configured and connected";
  case EnumerationState::ConfiguredNotConnected:
    return "Configured not connected";
  case EnumerationState::ConnectedNotConfigured:
    return "Connected not configured";
  default:
    return "Unknown";
  }
}

bool EnumerationInformation::isUndefined() const noexcept {
  static const EnumerationInformation undefined_ref = EnumerationInformation{};
  return *this == undefined_ref;
}

EnumerationInformation EnumerationInformation::fromBuffer(const std::vector<uint8_t>& buffer) {
  EnumerationInformation info;
  if (buffer.size() >= 10) {
    info.idx     = static_cast<unsigned int>(buffer[1]);
    info.type    = static_cast<SensorBoardType>(buffer[2]);
    info.version = Version{ buffer[3], buffer[4], buffer[5] };
    info.hash    = CommitHash::fromBits(buffer[6], buffer[7], buffer[8], buffer[9]);
  }
  return info;
}

bool operator==(const EnumerationInformation& lhs, unsigned int rhs) noexcept {
  return lhs.idx == rhs;
}

bool operator==(const EnumerationInformation& lhs, const EnumerationInformation& rhs) noexcept {
  return lhs.idx == rhs.idx;
}

bool operator<(const EnumerationInformation& lhs, const EnumerationInformation& rhs) noexcept {
  return lhs.idx < rhs.idx;
}

} // namespace sensor

} // namespace eduart