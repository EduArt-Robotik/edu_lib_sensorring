// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   InterfaceParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Per-interface-type configuration parameters.
 * @date   2026-06-15
 */

#pragma once

#include <string>

#include "sensorring/platform/SensorringBaseExport.hpp"

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @struct InterfaceParams
 * @brief Base class for interface configuration parameters.
 */
struct SENSORRING_BASE_EXPORT InterfaceParams {
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
struct SENSORRING_BASE_EXPORT CanParams : InterfaceParams {

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
struct SENSORRING_BASE_EXPORT SocketCanParams : CanParams {

  SocketCanParams() = default;
  SocketCanParams(std::string interface_name, bool respond_with_brs = false, unsigned int data_bitrate = 0, float data_sample_point = 0.0f)
      : CanParams(std::move(interface_name), respond_with_brs, data_bitrate, data_sample_point) {}
};

/**
 * @struct UsbTingoParams
 * @brief Configuration parameters for a USBtingo CAN adapter.
 */
struct SENSORRING_BASE_EXPORT UsbTingoParams : CanParams {

  UsbTingoParams() = default;
  UsbTingoParams(std::string interface_name, bool respond_with_brs = false, unsigned int data_bitrate = 0, float data_sample_point = 0.0f)
      : CanParams(std::move(interface_name), respond_with_brs, data_bitrate, data_sample_point) {}
};

/**
 * @struct Ethernet2CanParams
 * @brief Configuration for one CAN bus routed through an Ethernet gateway.
 *
 * Each CAN bus uses a separate UDP interface and bus index. Replies are sent
 * to the local UDP port from which the request originated.
 */
struct SENSORRING_BASE_EXPORT Ethernet2CanParams : CanParams {
  /// IPv4 address or hostname of the Ethernet gateway.
  std::string host;

  /// UDP port on the gateway.
  unsigned short port = 5000;

  /// Local UDP port; zero lets the operating system choose an ephemeral port.
  unsigned short local_port = 0;

  /// CAN bus index understood by the gateway.
  unsigned char can_bus_index = 0;

  Ethernet2CanParams() = default;
  Ethernet2CanParams(std::string gateway_host, unsigned short gateway_port, unsigned char bus_index, unsigned short host_port = 0)
      : CanParams(gateway_host)
      , host(std::move(gateway_host))
      , port(gateway_port)
      , local_port(host_port)
      , can_bus_index(bus_index) {}

  /// Stable identifier distinguishing CAN buses on the same gateway.
  std::string interfaceName() const {
    return host + ":" + std::to_string(port) + "/" + std::to_string(can_bus_index) + ":" + std::to_string(local_port);
  }
};

} // namespace com

} // namespace sensorring

} // namespace eduart
