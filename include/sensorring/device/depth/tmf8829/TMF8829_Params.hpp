// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   TMF8829_Params.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the TMF8829 Time-of-Flight sensor.
 * @date   2026-05-08
 */

#pragma once

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
 * @struct TMF8829_Params
 * @brief Parameter structure of the TMF8829 sensor of a sensor board.
 */
struct SENSORRING_EXPORT TMF8829_Params : public DeviceParams {
  /// Default constructor; sets the maximum measurement rate to 30 Hz.
  TMF8829_Params() { max_rate_hz = 30.0; }

  /// Resolution mode of the sensor. See TMF8829 datasheet for details on the different modes.
  ResolutionMode resolution_mode = ResolutionMode::RES_8X8;

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
