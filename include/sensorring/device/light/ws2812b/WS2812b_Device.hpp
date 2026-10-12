// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   WS2812b_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for WS2812b LED strip devices
 * @date   2025-02-11
 */

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

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
class SENSORRING_EXPORT WS2812b_Device : public Light {
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
   * @brief Re-apply runtime configuration after board reset.
   * @return true on success.
   */
  bool configure() override;

  /**
   * @brief Set the distance map range in meters and verify it by reading it back.
   * @return true if both values match within float32 rounding tolerance.
   */
  bool setDistanceMapRange(double lower_m, double upper_m);

  /**
   * @brief Read the current distance map range from the board in meters.
   * @param[out] lower_m Lower distance threshold, unchanged on failure.
   * @param[out] upper_m Upper distance threshold, unchanged on failure.
   * @return true if a valid response was received.
   */
  bool getDistanceMapRange(double& lower_m, double& upper_m);

  /**
   * @brief Set distance map peak channel brightness and verify it by reading it back.
   * @param[in] brightness Integer brightness in [0, 255].
   * @return true if the returned brightness matches exactly.
   */
  bool setDistanceMapMaxBrightness(std::int64_t brightness);

  /**
   * @brief Read the current distance map peak channel brightness from the board.
   * @param[out] brightness Current brightness, unchanged on failure.
   * @return true if a valid response was received.
   */
  bool getDistanceMapMaxBrightness(std::int64_t& brightness);

  /**
   * @brief Get the parameters used to configure this device.
   * @return Reference to the internal WS2812b parameter struct.
   */
  const WS2812b_Params& getParams() const override { return _params; }

  // --- Light interface overrides ---

  /**
   * @brief Set the light mode and color. Enqueues a per-device CAN command into the action queue.
   * @param[in] mode Light mode to apply.
   * @param[in] r    Red channel (0-255).
   * @param[in] g    Green channel (0-255).
   * @param[in] b    Blue channel (0-255).
   */
  void setLight(LightMode mode, std::uint8_t r, std::uint8_t g, std::uint8_t b) override;

  /**
   * @brief Synchronize pending light updates on all WS2812b devices.
   */
  static void syncLight();

protected:
  /// Construct with a non-owning communication interface supplied by a subclass.
  WS2812b_Device(WS2812b_Params params, com::ComInterface* interface, unsigned int idx);

private:
  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) override;

  bool requestParameter(std::uint8_t command, std::vector<uint8_t>& data);

  static constexpr std::chrono::milliseconds GET_PARAMETER_SLEEP   = std::chrono::milliseconds(10);
  static constexpr std::chrono::milliseconds GET_PARAMETER_TIMEOUT = std::chrono::milliseconds(100);

  std::mutex _parameter_mutex;
  std::mutex _state_mutex;
  std::atomic<bool> _got_update   = false;
  std::uint8_t _pending_parameter = 0;
  std::vector<uint8_t> _parameter_data;

  WS2812b_Params _params;

  struct Setting {
    LightMode mode     = LightMode::Off;
    std::uint8_t red   = 0;
    std::uint8_t green = 0;
    std::uint8_t blue  = 0;
  } _last_setting;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
