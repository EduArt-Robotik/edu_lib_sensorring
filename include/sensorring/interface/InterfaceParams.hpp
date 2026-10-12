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
 * @struct CanParams
 * @brief Shared parameters for a CAN interfaces.
 */
struct SENSORRING_EXPORT CanParams : InterfaceParams {

  /// Enable CAN FD bit rate switching for the data phase when the sensor boards send messages to the host.
  bool respond_with_brs = false;

  /// Data bitrate in bits per second in the range [1000000, 8000000]. Only relevant if one of the brs settings is true. Default 0 uses the same bitrate for arbitration and data phase (1 Mbps).
  unsigned int data_bitrate = 0;

  /// Data sample point in the range [0, 1]. Optional parameter for fine-tuning. Default 0 uses the CAN controller's default sample point.
  float data_sample_point = 0.0f;

  /**
   * @brief Enable CAN FD bit rate switching for the data phase when the host sends messages to the sensor boards.
   * The send_with_brs parameter is protected by this function to prevent accidental misuse.
   * Be sure that the host CAN controller supports TDC (transmitter delay compensation),
   * otherwise communication may become unreliable.
   * @param enable Whether to enable BRS for sending messages.
   */
  void send_with_brs(bool enable) { _send_with_brs = enable; }

  /**
   * @brief Get whether CAN FD bit rate switching for the data phase is enabled when the host sends messages to the sensor boards.
   * @return Whether BRS for sending messages is enabled.
   */
  bool send_with_brs() const { return _send_with_brs; }

  CanParams() = default;
  CanParams(std::string interface_name, bool respond_with_brs = false, unsigned int data_bitrate = 0, float data_sample_point = 0.0f)
      : InterfaceParams(std::move(interface_name))
      , respond_with_brs(respond_with_brs)
      , data_bitrate(data_bitrate)
      , data_sample_point(data_sample_point) {}

private:
  /// Enable CAN FD bit rate switching for the data phase when the host sends messages to the sensor boards.
  /// The host must support TDC (transmitter delay compensation) for this to work reliably.
  /// Receiving messages with BRS enabled works regardless of TDC support.
  bool _send_with_brs = false;
};

/**
 * @struct SocketCanParams
 * @brief Configuration parameters for a Linux SocketCAN interface.
 */
struct SENSORRING_EXPORT SocketCanParams : CanParams {

  SocketCanParams() = default;
  SocketCanParams(std::string interface_name, bool respond_with_brs = false, unsigned int data_bitrate = 0, float data_sample_point = 0.0f)
      : CanParams(std::move(interface_name), respond_with_brs, data_bitrate, data_sample_point) {}
};

/**
 * @struct UsbTingoParams
 * @brief Configuration parameters for a USBtingo CAN adapter.
 */
struct SENSORRING_EXPORT UsbTingoParams : CanParams {

  UsbTingoParams() = default;
  UsbTingoParams(std::string interface_name, bool respond_with_brs = false, unsigned int data_bitrate = 0, float data_sample_point = 0.0f)
      : CanParams(std::move(interface_name), respond_with_brs, data_bitrate, data_sample_point) {}
};

} // namespace com

} // namespace sensorring

} // namespace eduart
