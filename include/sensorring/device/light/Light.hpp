// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Light.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for light/LED actuator devices.
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
 * Users call setColor() / setMode() to update the desired state. Concrete
 * implementations enqueue self-contained CAN commands into the IDevice action
 * queue, which the state machine drains and executes during the device_actions cycle.
 */
class SENSORRING_EXPORT Light {
public:
  virtual ~Light() = default;

  /**
   * @brief Set the desired RGB color.
   * @param[in] r Red channel (0-255).
   * @param[in] g Green channel (0-255).
   * @param[in] b Blue channel (0-255).
   */
  virtual void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) = 0;

  /**
   * @brief Set the desired light mode.
   * @param[in] mode Light mode to apply.
   */
  virtual void setMode(LightMode mode) = 0;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
