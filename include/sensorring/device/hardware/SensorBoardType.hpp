// Copyright (c) 2026 EduArt Robotik GmbH
/**
 * @file   Parameter.hpp
 * @author EduArt Robotik GmbH
 * @brief  Board type enum for sensor board hardware variants (matches firmware).
 * @date   2026-02-17
 */

#pragma once

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

} // namespace device

} // namespace eduart
