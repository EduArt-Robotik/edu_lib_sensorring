#include "sensorring/device/EnumerationInformation.hpp"

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

bool Version::operator==(const Version& other) const noexcept {
  return major == other.major && minor == other.minor && patch == other.patch;
}

bool Version::operator<(const Version& other) const noexcept {
  if (major != other.major)
    return major < other.major;
  if (minor != other.minor)
    return minor < other.minor;
  return patch < other.patch;
}

CommitHash CommitHash::fromBits(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept {
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

bool CommitHash::operator==(const CommitHash& other) const noexcept {
  return hash == other.hash;
}

namespace device {

std::string toString(ConnectionState state) {
  switch (state) {
  case ConnectionState::Undefined:
    return "Undefined";
  case ConnectionState::Connected:
    return "Connected";
  case ConnectionState::Unconnected:
    return "Unconnected";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const ConnectionState state) noexcept {
  return os << toString(state);
}

std::string toString(ConfigurationState state) {
  switch (state) {
  case ConfigurationState::Undefined:
    return "Undefined";
  case ConfigurationState::Configured:
    return "Configured";
  case ConfigurationState::Unconfigured:
    return "Unconfigured";
  default:
    return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const ConfigurationState state) noexcept {
  return os << toString(state);
}

bool EnumerationInformation::isUndefined() const noexcept {
  static const EnumerationInformation undefined_ref = EnumerationInformation{};
  return *this == undefined_ref;
}

EnumerationInformation EnumerationInformation::fromBuffer(const std::vector<uint8_t>& buffer) {
  EnumerationInformation info;
  if (buffer.size() >= 12) {
    info.idx            = static_cast<unsigned int>(buffer[1]);
    info.type           = static_cast<SensorBoardType>(buffer[2]);
    info.version        = Version{ buffer[3], buffer[4], buffer[5] };
    info.hash           = CommitHash::fromBits(buffer[6], buffer[7], buffer[8], buffer[9]);
    info.device_options = static_cast<std::uint16_t>((static_cast<std::uint16_t>(buffer[10]) << 8) | static_cast<std::uint16_t>(buffer[11]));

    for (int i = 0; i < 16; i++) {
      auto device_type = static_cast<DeviceType>(i);
      if (info.hasDevice(device_type)) {
        info.devices.push_back(device_type);
      }
    }
  }
  return info;
}

bool EnumerationInformation::hasDevice(DeviceType type) const noexcept {
  if (type == DeviceType::UNDEFINED) {
    return false;
  }
  const auto bit = static_cast<std::uint16_t>(1u) << static_cast<std::uint8_t>(type);
  return (device_options & bit) != 0u;
}

bool EnumerationInformation::operator==(unsigned int other) const noexcept {
  return idx == other;
}

bool EnumerationInformation::operator==(const EnumerationInformation& other) const noexcept {
  return idx == other.idx;
}

bool EnumerationInformation::operator<(const EnumerationInformation& other) const noexcept {
  return idx < other.idx;
}

std::string EnumerationInformation::toString() const {
  std::ostringstream ss;
  ss << "Board " << idx << "\n";
  ss << "    Type:           " << type << "\n";
  ss << "    Connection:     " << state << "\n";
  ss << "    Configuration:  " << config_state << "\n";

  if (state == ConnectionState::Connected) {
    ss << "    FW revision:    " << version << " (" << hash << ")" << "\n";
  }

  if (!devices.empty()) {
    ss << "    Devices (HW):   ";
    for (std::size_t i = 0; i < devices.size(); ++i) {
      if (i > 0)
        ss << ", ";
      ss << devices[i];
    }
    ss << "\n";
  }

  if (!configured_devices.empty() && configured_devices != devices) {
    ss << "    Devices (used): ";
    for (std::size_t i = 0; i < configured_devices.size(); ++i) {
      if (i > 0)
        ss << ", ";
      ss << configured_devices[i];
    }
    ss << "\n";
  }

  return ss.str();
}

} // namespace device

} // namespace eduart