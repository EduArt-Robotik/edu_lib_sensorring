// Copyright (c) 2025 EduArt Robotik GmbH

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @class IDevice
 * @brief Thin base interface for all concrete devices in the SensorRing.
 *
 * IDevice serves purely as a common polymorphic base for all device implementations.
 * All device-specific behaviour is accessed through the concrete device types (i.e. by dynamic_cast).
 */
struct SENSORRING_EXPORT IDevice {
  virtual ~IDevice() = default;
};

} // namespace device

} // namespace eduart