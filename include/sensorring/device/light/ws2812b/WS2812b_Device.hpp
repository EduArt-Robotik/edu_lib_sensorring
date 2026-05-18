// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   WS2812b_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for WS2812b LED strip devices
 * @date   2025-02-11
 */

#pragma once

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/light/Light.hpp"
#include "sensorring/device/light/LightMode.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class WS2812b_Device
 * @brief  Device wrapper for WS2812b LED strips controlled via the sensorring bus.
 */
class SENSORRING_EXPORT WS2812b_Device : public BaseDevice, public Light {
public:
  /**
   * @brief Construct a new WS2812b device instance.
   * @param[in] params    LED strip configuration parameters.
   * @param[in] interface Communication interface used to talk to the device.
   * @param[in] idx       Index of the device on the bus.
   */
  WS2812b_Device(WS2812b_Params params, com::ComInterfaceID interface, unsigned int idx);

  ~WS2812b_Device() = default;

  /**
   * @brief Get the parameters used to configure this device.
   * @return Reference to the internal WS2812b parameter struct.
   */
  const WS2812b_Params& getParams() const { return _params; }

  // --- Light interface overrides ---

  /**
   * @brief Set the RGB color. Enqueues a CAN command into the device action queue.
   * @param[in] r Red channel (0–255).
   * @param[in] g Green channel (0–255).
   * @param[in] b Blue channel (0–255).
   */
  void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) override;

  /**
   * @brief Set the light mode. Enqueues a CAN command into the device action queue.
   * @param[in] mode Light mode to apply.
   */
  void setMode(LightMode mode) override;

  /**
   * @brief Re-apply runtime light synchronization and default mode after board reset.
   * @return true on success.
   */
  bool configure() override;

  // Simple static helpers to control all WS2812b devices on the bus.
  /**
   * @brief Set light mode and color for all WS2812b devices on the bus.
   * @param[in] mode  Light mode to apply.
   * @param[in] red   Red channel value.
   * @param[in] green Green channel value.
   * @param[in] blue  Blue channel value.
   * @return true on success.
   */
  static bool setLight(LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);
  /**
   * @brief Synchronize pending light updates on all WS2812b devices.
   * @return true on success.
   */
  static bool syncLight();

private:
  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) override;

  const WS2812b_Params _params;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
