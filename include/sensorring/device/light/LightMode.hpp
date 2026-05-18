// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   LightMode.hpp
 * @author EduArt Robotik GmbH
 * @brief  Light mode definition
 * @date   2025-11-20
 */

#pragma once

#include <cstdint>

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @enum LightMode
 * @brief Light mode enumeration.
 */
enum class LightMode : std::uint8_t {
  Off             = 0x02, ///< All LEDs off.
  Dimmed          = 0x03, ///< Low-brightness white illumination.
  HighBeam        = 0x04, ///< Full-brightness white illumination.
  FlashAll        = 0x05, ///< All LEDs flash simultaneously.
  FlashLeft       = 0x06, ///< Left-side LEDs flash (turn signal).
  FlashRight      = 0x07, ///< Right-side LEDs flash (turn signal).
  Pulsation       = 0x08, ///< Brightness pulses in and out.
  Rotation        = 0x09, ///< Rotation effect per board.
  Running         = 0x0A, ///< Running-light animation.
  MapDistance     = 0x0B, ///< Color encodes proximity distance data.
  FixedColor      = 0x0C, ///< Solid user-defined RGB color.
  PulsationColor  = 0x0D, ///< Pulsation with user-defined RGB color.
  IndividualColor = 0x0E  ///< Per-LED individually addressable color.
};

} // namespace device

} // namespace sensorring

} // namespace eduart