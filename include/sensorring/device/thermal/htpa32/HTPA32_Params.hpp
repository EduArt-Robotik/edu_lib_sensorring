// Copyright (c) 2026 EduArt Robotik GmbH
//
// Parameter structure for the HTPA32 thermal sensor.

#pragma once

#include <string>

#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct HTPA32_Params
 * @brief Parameter structure of the thermal sensor of a sensor board.
 */
struct SENSORRING_EXPORT HTPA32_Params : public DeviceParams {
  HTPA32_Params() { max_rate_hz = 5.0; }

  /// Minimal temperature in degree celsius used for color mapping of the thermal images. Only used when auto_min_max is set to false.
  double t_min_deg_c = 20;

  /// Maximal temperature in degree celsius used for color mapping of the thermal images. Only used when auto_min_max is set to false.
  double t_max_deg_c = 30;

  /// Enable automatic color scaling of the thermal images using the coldest and the hottest temperature in each image.
  bool auto_min_max = true;

  /// Save the thermal sensors eeprom content to a local file to only require a transfer once.
  bool use_eeprom_file = false;

  /// Save the calibration data for the thermal sensor to a local file to only require the calibration procedure once.
  bool use_calibration_file = false;

  /// Directory of the eeprom file. The user requires read and write access to this directory.
  std::string eeprom_dir = "";

  /// Directory of the calibration data file.  The user requires read and write access to this directory.
  std::string calibration_dir = "";

  /// Orientation of the sensor board. Used to flip the image upside down.
  Orientation orientation = Orientation::None;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
