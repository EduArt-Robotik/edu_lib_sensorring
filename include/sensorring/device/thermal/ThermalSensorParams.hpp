// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ThermalSensorParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the ThermalSensorParams.
 * @date   2026-02-19
 */

#pragma once

#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct ThermalSensorParams
 * @brief Parameter structure of the ThermalSensorParams.
 */
struct SENSORRING_EXPORT ThermalSensorParams : public DeviceParams {
  /// Destructor
  virtual ~ThermalSensorParams() = default;

  /// Minimal temperature in degree celsius used for color mapping of the thermal images. Only used when auto_min_max is set to false.
  double t_min_deg_c = 20;

  /// Maximal temperature in degree celsius used for color mapping of the thermal images. Only used when auto_min_max is set to false.
  double t_max_deg_c = 30;

  /// Enable automatic color scaling of the thermal images using the coldest and the hottest temperature in each image.
  bool auto_min_max = true;
};

} // namespace device

} // namespace sensorring

} // namespace eduart