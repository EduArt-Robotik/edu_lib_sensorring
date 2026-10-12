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

#include "sensorring/board/SensorBoardType.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

/**
 * @struct Version
 * @brief Semantic version (major, minor, patch) for firmware or board.
 */
struct SENSORRING_EXPORT Version {
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
   * @param[in] other Version to compare with.
   * @return true if equal.
   */
  bool operator==(const Version& other) const noexcept;

  /**
   * @brief Lexicographic less-than comparison (major, minor, patch).
   * @param[in] other Version to compare with.
   * @return true if *this < other.
   */
  bool operator<(const Version& other) const noexcept;
};

/**
 * @struct CommitHash
 * @brief 32-bit commit hash (e.g. from firmware) with byte-wise construction and string output.
 */
struct SENSORRING_EXPORT CommitHash {
  /// 32-bit hash value.
  std::uint32_t hash = 0;

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
   * @param[in] other CommitHash to compare with.
   * @return true if equal.
   */
  bool operator==(const CommitHash& other) const noexcept;

  /**
   * @brief Assign commit hash from a 32-bit integer.
   * @param[in] other 32-bit integer representing the commit hash.
   * @return true if equal.
   */
  bool operator=(const std::uint32_t& other) noexcept;
};

namespace board {

/**
 * @enum ConnectionState
 * @brief Physical connection state of a board after enumeration.
 */
enum class ConnectionState {
  /// Connection state has not been determined yet.
  Undefined,
  /// Board is physically connected.
  Connected,
  /// Board is not physically connected.
  Unconnected
};

/**
 * @enum ConfigurationState
 * @brief Whether a board or device was declared by the user via the factory.
 */
enum class ConfigurationState {
  /// Configuration state has not been determined yet.
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
struct SENSORRING_EXPORT EnumerationInformation {
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
  std::vector<device::DeviceType> devices;

  /// Configuration/connection state after enumeration.
  ConnectionState state = ConnectionState::Undefined;

  /// Configuration state set by the factory during build().
  ConfigurationState config_state = ConfigurationState::Undefined;

  /// Device types that were instantiated by the factory (subset of devices).
  std::vector<device::DeviceType> configured_devices;

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
  bool hasDevice(device::DeviceType type) const noexcept;

  /**
   * @brief Compare enumeration index to an integer.
   * @param[in] other Index to compare with idx.
   * @return true if idx == other.
   */
  bool operator==(unsigned int other) const noexcept;

  /**
   * @brief Compare two EnumerationInformation values for equality.
   * @param[in] other Value to compare with.
   * @return true if all fields are equal.
   */
  bool operator==(const EnumerationInformation& other) const noexcept;

  /**
   * @brief Order by index for sorting.
   * @param[in] other Value to compare with.
   * @return true if idx < other.idx.
   */
  bool operator<(const EnumerationInformation& other) const noexcept;

  /**
   * @brief Format this board's enumeration info as a human-readable string.
   * @return Multi-line string describing the board.
   */
  std::string toString() const;
};

} // namespace board

} // namespace sensorring

} // namespace eduart