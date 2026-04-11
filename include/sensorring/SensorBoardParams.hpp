// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoardParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure of a sensor board
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/device/hardware/SensorBoardType.hpp"
#include "sensorring/math/Math.hpp"

namespace eduart {

namespace sensorring {

namespace device {
/**
 * @struct SensorBoardParams
 * @brief Parameter structure of a sensor board. A sensor board is one circuit board.
 */
struct SensorBoardParams {
  /// Hardware board type. When set to Undefined, the board is created with all supported device types (backward compatibility).
  SensorBoardType board_type = SensorBoardType::Undefined;

  /// Rotation part of the sensors pose. The rotation is applied in the order Roll(x) - Pitch(y) - Yaw(z). Values: Euler angles in degrees
  math::Vector3 rotation = { 0, 0, 0 };

  /// Translation part of the sensor pose. Values: XYZ coordinates in meters.
  math::Vector3 translation = { 0, 0, 0 };
};

} // namespace device

} // namespace sensorring

} // namespace eduart