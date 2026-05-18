// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   AnyDeviceParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Category wildcard params for use with SensorRingFactory::expectBoard().
 *         Use these instead of a concrete device params type when you want to match
 *         any device belonging to a sensor category (depth / thermal / light).
 * @date   2026-05-08
 */

#pragma once

#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {
namespace sensorring {
namespace device {

/// Matches any depth sensor (VL53L8CX, TMF8829, ...).
struct SENSORRING_EXPORT AnyDepthSensor_Params : public DeviceParams {};

/// Matches any thermal sensor (HTPA32, ...).
struct SENSORRING_EXPORT AnyThermalSensor_Params : public DeviceParams {};

/// Matches any light device (WS2812b, ...).
struct SENSORRING_EXPORT AnyLight_Params : public DeviceParams {};

}}} // namespaces
