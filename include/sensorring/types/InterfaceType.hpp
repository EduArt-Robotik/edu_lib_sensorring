// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   InterfaceType.hpp
 * @author EduArt Robotik GmbH
 * @brief  Interface type definition
 * @date   2024-11-22
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

} // namespace eduart