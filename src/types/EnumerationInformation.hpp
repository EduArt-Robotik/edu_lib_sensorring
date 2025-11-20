#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "boardmanager/SensorBoardManager.hpp"

namespace eduart {

struct Version {
  unsigned int major = 0;
  unsigned int minor = 0;
  unsigned int patch = 0;

  std::string toString() const;

  friend std::ostream& operator<<(std::ostream& os, const Version& v) noexcept;

  friend bool operator==(const Version& lhs, const Version& rhs) noexcept;
};

struct CommitHash {
  std::uint32_t hash = 0;

  static inline CommitHash fromBits(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept;

  std::string toString() const;

  friend std::ostream& operator<<(std::ostream& os, const CommitHash& ch) noexcept;

  friend bool operator==(const CommitHash& lhs, const CommitHash& rhs) noexcept;
};

namespace sensor {

enum class EnumerationState {
  Undefined,
  ConfiguredAndConnected,
  ConfiguredNotConnected,
  ConnectedNotConfigured
};

std::string toString(EnumerationState state);

struct EnumerationInformation {
  unsigned int idx       = 0;
  Version version        = {};
  CommitHash hash        = {};
  SensorBoardType type   = SensorBoardType::Undefined;
  EnumerationState state = EnumerationState::Undefined;

  bool isUndefined() const noexcept;

  static EnumerationInformation fromBuffer(const std::vector<uint8_t>& buffer);

  friend bool operator==(const EnumerationInformation& lhs, unsigned int rhs) noexcept;

  friend bool operator==(const EnumerationInformation& lhs, const EnumerationInformation& rhs) noexcept;

  friend bool operator<(const EnumerationInformation& lhs, const EnumerationInformation& rhs) noexcept;
};

} // namespace sensor

} // namespace eduart