// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   HTPA32_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the HTPA32 thermal sensor.
 * @date   2026-05-08
 */

#pragma once

#include <string>

#include "sensorring/device/thermal/ThermalSensorParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct HTPA32_Params
 * @brief Parameter structure of the thermal sensor of a sensor board.
 */
struct SENSORRING_EXPORT HTPA32_Params : public ThermalSensorParams {

  /// @brief Default constructor; sets the maximum measurement rate to 5 Hz.
  HTPA32_Params() { max_rate_hz = 5.0; }

  /// @brief Initializes the HTPA32_Params from a ThermalSensorParams instance.
  HTPA32_Params(const ThermalSensorParams& params)
      : ThermalSensorParams{ params } { max_rate_hz = 5.0; };

  /// Save the thermal sensors eeprom content to a local file to only require a transfer once.
  bool use_eeprom_file = false;

  /// Save the calibration data for the thermal sensor to a local file to only require the calibration procedure once.
  bool use_calibration_file = false;

  /// Directory of the eeprom file. The user requires read and write access to this directory.
  std::string eeprom_dir = "";

  /// Directory of the calibration data file.  The user requires read and write access to this directory.
  std::string calibration_dir = "";
};

} // namespace device

} // namespace sensorring

} // namespace eduart
