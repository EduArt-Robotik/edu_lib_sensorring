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

enum class SENSORRING_API LightMode {
  Off,
  Dimmed,
  HighBeam,
  FlashAll,
  FlashLeft,
  FlashRight,
  Pulsation,
  Rotation,
  Running,
  MapDistance
};

} // namespace light

} // namespace eduart