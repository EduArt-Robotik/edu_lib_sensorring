// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   EnumerationInformation.hpp
 * @author EduArt Robotik GmbH
 * @brief  Version, commit hash, and board enumeration info reported by hardware.
 * @date   2025-02-19
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/hardware/SensorBoardType.hpp"

namespace eduart {

/**
 * @struct Version
 * @brief Semantic version (major, minor, patch) for firmware or board.
 */
struct Version {
  /// Major version number.
  unsigned int major = 0;
  /// Minor version number.
  unsigned int minor = 0;
  /// Patch version number.
  unsigned int patch = 0;

  /**
   * @brief Format version as "major.minor.patch".
   * @return Version string.
   */
  std::string toString() const;

  /**
   * @brief Stream version as "major.minor.patch".
   * @param[in] os Output stream.
   * @param[in] v Version to print.
   * @return Reference to os.
   */
  friend std::ostream& operator<<(std::ostream& os, const Version& v) noexcept;

  /**
   * @brief Compare two versions for equality.
   * @param[in] lhs First version.
   * @param[in] rhs Second version.
   * @return true if equal.
   */
  friend bool operator==(const Version& lhs, const Version& rhs) noexcept;

  /**
   * @brief Lexicographic less-than comparison (major, minor, patch).
   * @param[in] lhs First version.
   * @param[in] rhs Second version.
   * @return true if lhs < rhs.
   */
  friend bool operator<(const Version& lhs, const Version& rhs) noexcept;
};

/**
 * @struct CommitHash
 * @brief 32-bit commit hash (e.g. from firmware) with byte-wise construction and string output.
 */
struct CommitHash {
  /// 32-bit hash value.
  std::uint32_t hash = 0;

  /**
   * @brief Build a CommitHash from four bytes (e.g. from enumeration response).
   * @param[in] a First byte.
   * @param[in] b Second byte.
   * @param[in] c Third byte.
   * @param[in] d Fourth byte.
   * @return CommitHash with hash = (a<<24)|(b<<16)|(c<<8)|d.
   */
  static inline CommitHash fromBits(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept;

  /**
   * @brief Format hash as hex string.
   * @return Hex representation of hash.
   */
  std::string toString() const;

  /**
   * @brief Stream hash as hex string.
   * @param[in] os Output stream.
   * @param[in] ch CommitHash to print.
   * @return Reference to os.
   */
  friend std::ostream& operator<<(std::ostream& os, const CommitHash& ch) noexcept;

  /**
   * @brief Compare two commit hashes for equality.
   * @param[in] lhs First CommitHash.
   * @param[in] rhs Second CommitHash.
   * @return true if equal.
   */
  friend bool operator==(const CommitHash& lhs, const CommitHash& rhs) noexcept;
};

namespace device {

/**
 * @enum ConnectionState
 * @brief Physical connection state of a board after enumeration.
 */
enum class ConnectionState {
  Undefined,
  Connected,
  Unconnected
};

/**
 * @enum ConfigurationState
 * @brief Whether a board or device was declared by the user via the factory.
 */
enum class ConfigurationState {
  /// State has not been determined yet.
  Undefined,
  /// The board/device was declared by the user (expectBoard) and matched by the factory.
  Configured,
  /// The board/device was discovered by hardware but not declared by the user.
  Unconfigured
};

/**
 * @brief Convert connection state to a human-readable string.
 * @param[in] state Connection state.
 * @return String representation of state.
 */
SENSORRING_EXPORT std::string toString(ConnectionState state);

/**
 * @brief Convert configuration state to a human-readable string.
 * @param[in] state Configuration state.
 * @return String representation of state.
 */
SENSORRING_EXPORT std::string toString(ConfigurationState state);

/**
 * @brief Stream connection state as string.
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, const ConnectionState state) noexcept;

/**
 * @brief Stream configuration state as string.
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, const ConfigurationState state) noexcept;

/**
 * @struct EnumerationInformation
 * @brief Information reported by a board during enumeration: index, version, commit hash, board type, and state.
 */
struct EnumerationInformation {
  /// Board index.
  unsigned int idx = 0;

  /// Firmware/board version from enumeration response.
  Version version = {};

  /// Commit hash from enumeration response.
  CommitHash hash = {};

  /// Detected or configured board type.
  SensorBoardType type = SensorBoardType::Undefined;

  ///  Bitmask describing which devices are physically populated on the board.
  std::uint16_t device_options = 0;

  /// Devices on the board.
  std::vector<DeviceType> devices;

  /// Configuration/connection state after enumeration.
  ConnectionState state = ConnectionState::Undefined;

  /// Configuration state set by the factory during build().
  ConfigurationState config_state = ConfigurationState::Undefined;

  /// Device types that were instantiated by the factory (subset of devices).
  std::vector<DeviceType> configured_devices;

  /**
   * @brief Return true if this instance has not been filled from a valid enumeration response.
   * @return true if state/type are Undefined and idx is 0.
   */
  bool isUndefined() const noexcept;

  /**
   * @brief Parse enumeration info from the raw response buffer (e.g. CMD_ACTIVE_DEVICE_RESPONSE payload).
   * @param[in] buffer Raw bytes from the device.
   * @return Parsed EnumerationInformation.
   */
  static EnumerationInformation fromBuffer(const std::vector<uint8_t>& buffer);

  /**
   * @brief Check if a specific device type is reported as populated.
   * @param[in] type Device type to check.
   * @return true if the corresponding bit is set in device_options.
   */
  bool hasDevice(DeviceType type) const noexcept;

  /**
   * @brief Compare enumeration index to an integer.
   * @param[in] lhs Enumeration information.
   * @param[in] rhs Index to compare with lhs.idx.
   * @return true if lhs.idx == rhs.
   */
  friend bool operator==(const EnumerationInformation& lhs, unsigned int rhs) noexcept;

  /**
   * @brief Compare two EnumerationInformation values for equality.
   * @param[in] lhs First value.
   * @param[in] rhs Second value.
   * @return true if all fields are equal.
   */
  friend bool operator==(const EnumerationInformation& lhs, const EnumerationInformation& rhs) noexcept;

  /**
   * @brief Order by index for sorting.
   * @param[in] lhs First value.
   * @param[in] rhs Second value.
   * @return true if lhs.idx < rhs.idx.
   */
  friend bool operator<(const EnumerationInformation& lhs, const EnumerationInformation& rhs) noexcept;

  /**
   * @brief Format this board's enumeration info as a human-readable string.
   * @return Multi-line string describing the board.
   */
  std::string toString() const;
};

} // namespace device

} // namespace eduart