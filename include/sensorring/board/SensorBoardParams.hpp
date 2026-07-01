// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoardParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure of a sensor board
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/board/SensorBoardType.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace board {

/**
 * @enum Orientation
 * @brief Possible orientations of a sensor board. Used to rotate/mirror light animations and thermal images.
 */
enum class Orientation {
  None, ///< No specific orientation; default behaviour applies.
  Left, ///< Board is mounted on the left side; animations are mirrored horizontally.
  Right ///< Board is mounted on the right side; animations are used as-is.
};

/**
 * @brief Function to convert the Orientation enum class members to string
 * @param[in] mode to be converted to a string
 * @return Name of the orientation written out as string
 */
SENSORRING_EXPORT std::string toString(Orientation mode) noexcept;

/**
 * @brief  Output stream operator for the Orientation enum class members
 * @param[in] os output stream to write to
 * @param[in] mode to be printed as stream
 * @return Stream with the orientation name written out
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, Orientation mode) noexcept;

/**
 * @struct SensorBoardParams
 * @brief Parameter structure of a sensor board. A sensor board is one circuit board.
 */
struct SENSORRING_EXPORT SensorBoardParams {
  /// Hardware board type. When set to Undefined, the board is created with all supported device types (backward compatibility).
  SensorBoardType board_type = SensorBoardType::Undefined;

  /// Board orientation used by devices that require orientation-dependent processing.
  Orientation orientation = Orientation::None;

  /// Rotation part of the sensors pose. The rotation is applied in the order Roll(x) - Pitch(y) - Yaw(z). Values: Euler angles in degrees
  math::Vector3 rotation = { 0, 0, 0 };

  /// Translation part of the sensor pose. Values: XYZ coordinates in meters.
  math::Vector3 translation = { 0, 0, 0 };
};

} // namespace board

} // namespace sensorring

} // namespace eduart