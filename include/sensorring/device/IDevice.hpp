// Copyright (c) 2025 EduArt Robotik GmbH

#pragma once

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @class IDevice
 * @brief Thin base interface for all concrete devices in the SensorRing.
 *
 * In the simplified design, IDevice serves purely as a common polymorphic base
 * so that containers such as SensorRing and DeviceGroup can hold heterogeneous
 * devices via `IDevice*`. All device-specific behaviour is accessed through
 * the concrete device types (e.g. VL53L8CX_Device, HTPA32_Device, WS2812b_Device).
 */
struct SENSORRING_EXPORT IDevice {
  virtual ~IDevice() = default;
};

} // namespace device

} // namespace eduart