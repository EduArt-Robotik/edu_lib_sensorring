// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   InterfaceParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Per-interface-type configuration parameters.
 * @date   2026-06-15
 */

#pragma once

#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @struct InterfaceParams
 * @brief Base class for interface configuration parameters.
 */
struct SENSORRING_EXPORT InterfaceParams {
  virtual ~InterfaceParams() = default;

  /// Interface identifier (e.g. "can0" for SocketCAN, serial number for USBtingo).
  std::string name;

protected:
  InterfaceParams() = default;
  explicit InterfaceParams(std::string name)
      : name(std::move(name)) {}
};

/**
 * @struct SocketCanParams
 * @brief Configuration parameters for a Linux SocketCAN interface.
 */
struct SENSORRING_EXPORT SocketCanParams : InterfaceParams {
  /// Enable CAN FD bit rate switching for the data phase.
  bool enable_brs = false;

  SocketCanParams() = default;
  explicit SocketCanParams(std::string interface_name, bool enable_brs = false)
      : InterfaceParams(std::move(interface_name))
      , enable_brs(enable_brs) {}
};

/**
 * @struct UsbTingoParams
 * @brief Configuration parameters for a USBtingo CAN adapter.
 */
struct SENSORRING_EXPORT UsbTingoParams : InterfaceParams {
  /// Enable CAN FD bit rate switching for the data phase.
  bool enable_brs = false;

  UsbTingoParams() = default;
  explicit UsbTingoParams(std::string serial, bool enable_brs = false)
      : InterfaceParams(std::move(serial))
      , enable_brs(enable_brs) {}
};

} // namespace com

} // namespace sensorring

} // namespace eduart
