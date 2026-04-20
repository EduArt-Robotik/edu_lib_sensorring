// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Orientation.hpp
 * @author EduArt Robotik GmbH
 * @brief  Orientation enum shared across device types.
 */

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @enum Orientation
 * @brief Possible orientations of a sensor board. Used to rotate/mirror light animations and thermal images.
 */
enum class Orientation {
  Left,
  Right,
  None
};

} // namespace device

} // namespace sensorring

} // namespace eduart
