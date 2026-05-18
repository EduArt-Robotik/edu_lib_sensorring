// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   TMF8829_ResultFormat.hpp
 * @author EduArt Robotik GmbH
 * @brief  Result format structure for the TMF8829 Time-of-Flight sensor.
 * @date   2026-05-08
 */

#pragma once

#include <cstdint>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @struct TMF8829_ResultFormat
 * @brief Specifies which values are included in the TMF8829 measurement
 */
struct SENSORRING_EXPORT TMF8829_ResultFormat {
  bool full_noise          = false; ///< If true, the noise_strength value is no longer divided by the number of bins
  bool xtalk               = false; ///< If true, the xtalk value is included in the measurement; See TMF8829 datasheet for details
  bool noise_strength      = false; ///< If true, the noise_strength value is included in the measurement; See TMF8829 datasheet for details
  bool signal_strength     = false; ///< If true, the signal_strength value is included in the measurement; See TMF8829 datasheet for details
  std::uint8_t nr_of_peaks = 1;     ///< If non zero this is the maximum number of reported peaks per depth pixel. Valid values are 0, 1, 2, 3, 4.

  /**
   * @brief Calculate the size of a single point in the point cloud based on which values are included in the measurement
   * @return Size of a single point in bytes
   */
  std::size_t calculatePointSize() const;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
