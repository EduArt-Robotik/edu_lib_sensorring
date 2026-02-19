// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ComInterfaceID.hpp
 * @author EduArt Robotik GmbH
 * @brief  Interface type definition
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace com {

/**
 * @enum InterfaceType
 * @brief Type of the communication interface.
 */
enum class SENSORRING_EXPORT InterfaceType {
  UNDEFINED,
  SOCKETCAN,
  USBTINGO
};

/**
 * @struct InterfaceParams
 * @brief Parameters of a communication interface.
 */
struct SENSORRING_EXPORT ComInterfaceID {
  /// Type of the communication interface.
  InterfaceType type = InterfaceType::UNDEFINED;

  /// Name of the communication interface.
  std::string name = "";
};

} // namespace com

} // namespace eduart