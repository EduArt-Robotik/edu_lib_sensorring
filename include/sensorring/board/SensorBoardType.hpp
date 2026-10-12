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

namespace sensorring {

namespace board {

/**
 * @enum SensorBoardType
 * @brief Hardware variant of a sensor board.
 *
 * The numeric values match the board-type field reported by the sensor board firmware during enumeration.
 */
enum class SensorBoardType {
  Sidepanel = 0x00, ///< Sidepanel board.
  Headlight = 0x01, ///< Headlight board.
  Taillight = 0x02, ///< Taillight board.
  Minipanel = 0x03, ///< Minipanel board.
  Undefined = 0xff  ///< Board type could not be determined.
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

} // namespace board

} // namespace sensorring

} // namespace eduart
