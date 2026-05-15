// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   TMF8829_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the TMF8829 Time-of-Flight sensor.
 * @date   2026-05-08
 */

#pragma once

#include <cstdint>

#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @brief Supported resolution modes of the TMF8829 sensor.
 */
enum class ResolutionMode : std::uint8_t {
  RES_8X8                 = 0,
  RES_8X8_LONG_RANGE      = 1,
  RES_8X8_HIGH_ACCURACY   = 2,
  RES_16X16               = 3,
  RES_16X16_HIGH_ACCURACY = 4,
  RES_32X32               = 5,
  RES_32X32_HIGH_ACCURACY = 6,
  RES_48X32               = 7,
  RES_48X32_HIGH_ACCURACY = 8
};

/**
 * @brief Function to convert the ResolutionMode enum class members to string
 * @param[in] mode to be converted to a string
 * @return Name of the resolution mode written out as string
 */
SENSORRING_EXPORT std::string toString(ResolutionMode mode) noexcept;

/**
 * @brief  Output stream operator for the ResolutionMode enum class members
 * @param[in] os output stream to write to
 * @param[in] mode to be printed as stream
 * @return Stream with the resolution mode name written out
 */
SENSORRING_EXPORT std::ostream& operator<<(std::ostream& os, ResolutionMode mode) noexcept;

/**
 * @struct TMF8829_ResultFormat
 * @brief Specifies which values are included in the TMF8829 measurement
 */
struct TMF8829_ResultFormat {
  bool full_noise          = false; ///< If true, the noise_strength value is no longer divided by the number of bins
  bool xtalk               = false; ///< If true, the xtalk value is included in the measurement; See TMF8829 datasheet for details
  bool noise_strength      = false; ///< If true, the noise_strength value is included in the measurement; See TMF8829 datasheet for details
  bool signal_strength     = false; ///< If true, the signal_strength value is included in the measurement; See TMF8829 datasheet for details
  std::uint8_t nr_of_peaks = 1;     ///< If non zero this is the maximum number of reported peaks per depth pixel. Valid values are 0, 1, 2, 3, 4.
};

/**
 * @struct TMF8829_Params
 * @brief Parameter structure of the TMF8829 sensor of a sensor board.
 */
struct SENSORRING_EXPORT TMF8829_Params : public DeviceParams {
  /// @brief Default constructor; sets the maximum measurement rate to 30 Hz.
  TMF8829_Params() { max_rate_hz = 30.0; }

  ResolutionMode resolution_mode = ResolutionMode::RES_8X8; ///< Specifies the resolution mode of the sensor

  TMF8829_ResultFormat result_format; ///< Specifies which additional values are included in the measurement
};

} // namespace device

} // namespace sensorring

} // namespace eduart
