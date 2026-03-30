// Copyright (c) 2026 EduArt Robotik GmbH
/**
 * @file   SensorBoardType.hpp
 * @author EduArt Robotik GmbH
 * @brief  Board type enum for sensor board hardware variants (matches firmware).
 * @date   2026-02-17
 */

#pragma once

#include <ostream>
#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/** Numbers match the definition in the sensor board firmware. */
enum class SensorBoardType {
  Sidepanel = 0x00,
  Headlight = 0x01,
  Taillight = 0x02,
  Minipanel = 0x03,
  Undefined = 0xff
};

/**
 * @brief Format sensor board type as string.
 * @return Sensor board type string.
 */
SENSORRING_EXPORT std::string toString(SensorBoardType type) noexcept;

/**
 * @brief Stream sensor board type as string.
 * @param[in] os Output stream.
 * @param[in] type Sensor board type to print.
 * @return Reference to os.
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, const SensorBoardType type) noexcept;

} // namespace device

} // namespace eduart
