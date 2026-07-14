// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ComInterfaceID.hpp
 * @author EduArt Robotik GmbH
 * @brief  Interface type definition
 * @date   2026-02-19
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @enum InterfaceType
 * @brief Type of the communication interface.
 */
enum class InterfaceType {
  Undefined, ///< Type not yet assigned.
  SocketCan, ///< Linux SocketCAN interface (e.g. can0).
  UsbTingo   ///< USBtingo CAN adapter.
};

/**
 * @brief Format interface type as string.
 * @return Interface type string.
 */
SENSORRING_EXPORT std::string toString(InterfaceType type) noexcept;

/**
 * @brief Stream interface type as string.
 * @param[in] os Output stream.
 * @param[in] type Interface type to print.
 * @return Reference to os.
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, const InterfaceType type) noexcept;

/**
 * @struct ComInterfaceID
 * @brief Uniquely identifies a communication interface by type and name.
 */
struct ComInterfaceID {
  /// Type of the communication interface.
  InterfaceType type = InterfaceType::Undefined;

  /// Name of the communication interface.
  std::string name = "";

  /// Equality operator for ComInterfaceID.
  bool operator==(const ComInterfaceID& other) const;

  /// Inequality operator for ComInterfaceID.
  bool operator!=(const ComInterfaceID& other) const;
};

inline bool ComInterfaceID::operator==(const ComInterfaceID& other) const {
  return type == other.type && name == other.name;
}

inline bool ComInterfaceID::operator!=(const ComInterfaceID& other) const {
  return !(type == other.type && name == other.name);
}

} // namespace com

} // namespace sensorring

} // namespace eduart

#ifndef SWIG
namespace std {

/**
 * @struct std::hash<ComInterfaceID>
 * @brief Hash specialization for ComInterfaceID to enable use in unordered containers.
 */
template <> struct hash<eduart::sensorring::com::ComInterfaceID> {
  /**
   * @brief Compute hash value for a ComInterfaceID.
   * @param[in] id The interface ID to hash.
   * @return Combined hash of type and name.
   */
  std::size_t operator()(const eduart::sensorring::com::ComInterfaceID& id) const noexcept {
    auto h1 = std::hash<int>{}(static_cast<int>(id.type));
    auto h2 = std::hash<std::string>{}(id.name);
    return h1 ^ (h2 << 1);
  }
};

} // namespace std
#endif // SWIG