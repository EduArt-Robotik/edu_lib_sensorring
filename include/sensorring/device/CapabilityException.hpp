// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   CapabilityException.hpp
 * @author EduArt Robotik GmbH
 * @brief  Exception type for unsupported capability requests on IDevice.
 * @date   2025-02-06
 */

#pragma once

#include <stdexcept>
#include <string>
#include <typeindex>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @class CapabilityNotSupported
 * @brief Exception thrown when a requested capability type is not supported.
 */
struct SENSORRING_EXPORT CapabilityNotSupported : std::runtime_error {
  /// std::type_index of the unsupported capability.
  std::type_index capability_type;

  /**
   * @brief Construct exception for the given capability type.
   * @param[in] cap std::type_index of the unsupported capability.
   */
  CapabilityNotSupported(std::type_index cap)
      : std::runtime_error(std::string("Capability not supported: ") + cap.name())
      , capability_type(cap) {}
};

} // namespace device

} // namespace eduart