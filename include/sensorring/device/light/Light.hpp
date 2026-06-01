// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Light.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for light/LED actuator devices.
 * @date   2026-05-08
 */

#pragma once

#include <cstdint>

#include "sensorring/device/light/LightMode.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class Light
 * @brief Public interface for light/LED actuator devices.
 *
 * Users call setLight() to update the desired state. The concrete implementation
 * enqueues a self-contained CAN command into the IDevice action queue, which the
 * state machine drains and executes during the device_actions cycle.
 */
class SENSORRING_EXPORT Light {
public:
  /// @brief Virtual destructor.
  virtual ~Light() = default;

  /**
   * @brief Set the light mode and color.
   * @param[in] mode Light mode to apply. Not all modes require a color to be specified.
   * @param[in] r    Red channel (0-255).
   * @param[in] g    Green channel (0-255).
   * @param[in] b    Blue channel (0-255).
   */
  virtual void setLight(LightMode mode, std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0) = 0;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
