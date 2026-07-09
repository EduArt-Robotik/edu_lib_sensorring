// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Light.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for light/LED actuator devices.
 * @date   2026-05-08
 */

#pragma once

#include <cstdint>

#include "sensorring/device/Device.hpp"
#include "sensorring/device/light/LightMode.hpp"
#include "sensorring/device/light/LightParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class Light
 * @brief Base class for light/LED actuator devices.
 *
 * Inherits from Device (identity, pose, action queue).
 * Users call setLight() to update the desired state. The concrete implementation
 * enqueues a self-contained CAN command into the action queue, which the
 * state machine drains and executes during the device_actions cycle.
 */
class SENSORRING_EXPORT Light : public Device {
public:
  /**
   * @brief Construct a Light with Device params.
   * @param[in] id        Device identifier.
   * @param[in] interface Communication interface.
   * @param[in] target    Communication endpoint this device listens to.
   */
  Light(DeviceID id, com::ComInterface* interface, com::ComEndpoint target);

  /// @brief Virtual destructor.
  ~Light() override = default;

  /**
   * @brief Get the current light parameters.
   * @return Current Light parameters.
   */
  virtual const LightParams& getParams() const = 0;

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
