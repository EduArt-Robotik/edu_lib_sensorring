// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   TMF8829_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the TMF8829 Time-of-Flight sensor.
 * @date   2026-05-08
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "sensorring/device/depth/tmf8829/TMF8829_ResultFormat.hpp"
#include "sensorring/device/types/DeviceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {
/**
 * @brief Supported resolution modes of the TMF8829 sensor.
 */
enum class ResolutionMode : std::uint8_t {
  Res8x8               = 0,
  Res8x8LongRange      = 1,
  Res8x8HighAccuracy   = 2,
  Res16x16             = 3,
  Res16x16HighAccuracy = 4,
  Res32x32             = 5,
  Res32x32HighAccuracy = 6,
  Res48x32             = 7,
  Res48x32HighAccuracy = 8
};

/**
 * @brief Get the horizontal resolution (number of columns) for a given resolution mode.
 * @param[in] mode Resolution mode for which to get the horizontal resolution.
 * @return Number of columns in the TMF8829 frame for the specified resolution mode.
 */
unsigned int getXResolution(ResolutionMode mode);

/**
 * @brief Get the vertical resolution (number of rows) for a given resolution mode.
 * @param[in] mode Resolution mode for which to get the vertical resolution.
 * @return Number of rows in the TMF8829 frame for the specified resolution mode.
 */
unsigned int getYResolution(ResolutionMode mode);

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
 * @struct TMF8829_Params
 * @brief Parameter structure of the TMF8829 sensor of a sensor board.
 */
struct SENSORRING_EXPORT TMF8829_Params : public DeviceParams {
  /// Default constructor; sets the maximum measurement rate to 30 Hz.
  TMF8829_Params() { max_rate_hz = 30.0; }

  /// Resolution mode of the sensor. See TMF8829 datasheet for details on the different modes.
  ResolutionMode resolution_mode = ResolutionMode::Res8x8;

  /// Iterations setting of the sensor in kilo iteration per measurement. Refer to the TMF8829 datasheet for more details about this setting. Set to 0 for sensor default value.
  std::uint16_t k_iterations = 0;

  /// Result format of the measurement. Specifies which additional values are included in each measurement.
  TMF8829_ResultFormat result_format;

  /**
   * @brief Calculate the resulting frame size of the current parameters.
   *
   * In high resolution modes, the measurement is split across two frames.
   * This method returns the per-frame size.
   *
   * @return The size of the result frame in bytes.
   */
  std::size_t calculateResultFrameSize() const;

  /**
   * @brief Check if the combination of the specified parameters is valid.
   *
   * The maximum result size per frame is limited to 8192 bytes.
   * The result size is a function of the resolution mode (point count)
   * and the result format (point size).
   *
   * @return True if the parameters are valid, false otherwise.
   */
  bool isResultSizeValid() const;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
