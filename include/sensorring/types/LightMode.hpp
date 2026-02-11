// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   LightMode.hpp
 * @author EduArt Robotik GmbH
 * @brief  Light mode definition
 * @date   2025-11-20
 */

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace light {

enum class SENSORRING_EXPORT LightMode : std::uint8_t {
  Off             = 0x02,
  Dimmed          = 0x03,
  HighBeam        = 0x04,
  FlashAll        = 0x05,
  FlashLeft       = 0x06,
  FlashRight      = 0x07,
  Pulsation       = 0x08,
  Rotation        = 0x09,
  Running         = 0x0A,
  MapDistance     = 0x0B,
  FixedColor      = 0x0C,
  PulsationColor  = 0x0D,
  IndividualColor = 0x0E
};

} // namespace light

} // namespace eduart