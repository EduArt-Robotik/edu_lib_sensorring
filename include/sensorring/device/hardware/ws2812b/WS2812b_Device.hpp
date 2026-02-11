// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   WS2812b_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for WS2812b LED strip devices
 * @date   2026-02-11
 */

#pragma once

#include <memory>

#include "sensorring/Parameter.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/types/LightMode.hpp"

namespace eduart {

namespace device {

class WS2812b_DeviceImpl;

/**
 * @struct WS2812b_Device
 * @brief  Device wrapper for WS2812b LED strips controlled via the sensorring bus.
 */
struct WS2812b_Device : BaseDevice {
public:
  /**
   * @brief Construct a new WS2812b device instance.
   * @param[in] params    LED strip configuration parameters.
   * @param[in] interface Communication interface used to talk to the device.
   */
  WS2812b_Device(WS2812b_Params params, com::ComInterface* interface);
  /// Destructor
  ~WS2812b_Device();

  /**
   * @brief Get the parameters used to configure this device.
   * @return Reference to the internal WS2812b parameter struct.
   */
  const WS2812b_Params& getParams() const;

  // Simple static helpers to control all WS2812b devices on the bus.
  /**
   * @brief Set light mode and color for all WS2812b devices on the bus.
   * @param[in] mode  Light mode to apply.
   * @param[in] red   Red channel value.
   * @param[in] green Green channel value.
   * @param[in] blue  Blue channel value.
   * @return true on success.
   */
  static bool setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);
  /**
   * @brief Synchronize pending light updates on all WS2812b devices.
   * @return true on success.
   */
  static bool syncLight();

  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  friend class WS2812b_DeviceImpl;
  std::unique_ptr<WS2812b_DeviceImpl> _impl;
};

} // namespace device

} // namespace eduart
