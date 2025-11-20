// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   CustomTypes.hpp
 * @author EduArt Robotik GmbH
 * @brief  Custom types used by the sensorring library
 * @date   2025-11-20
 */

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace com {

enum class SENSORRING_API InterfaceType {
  UNDEFINED,
  SOCKETCAN,
  USBTINGO
};

} // namespace com

namespace sensor {

enum class SENSORRING_API SensorState {
  SensorInit,
  SensorOK,
  ReceiveError
};

} // namespace sensor

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