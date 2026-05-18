// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ThermalMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Structures for thermal measurements
 * @date   2025-11-20
 */

#pragma once

#include <chrono>
#include <cstdint>

#include "sensorring/measurement/Header.hpp"
#include "sensorring/measurement/Image.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

/**
 * @class  GrayscaleImage
 * @brief  Grayscale image with 1 channel and 8 bit color depth
 */
class SENSORRING_EXPORT GrayscaleImage : public ScalarImage<std::uint8_t, THERMAL_RESOLUTION> {};

/**
 * @class  FalseColorImage
 * @brief  False color image with 3 channels (red, green, blue) and 8 bit color depth
 */
class SENSORRING_EXPORT FalseColorImage : public RgbImage<std::uint8_t, THERMAL_RESOLUTION> {};

/**
 * @class  TemperatureImage
 * @brief  Pseudo image structure for the converted temperatures of a thermal image.
 *
 * Each pixel stores a temperature value in °C. Provides methods to convert to
 * visualization images (grayscale or false-color iron palette).
 */
class SENSORRING_EXPORT TemperatureImage : public ScalarImage<double, THERMAL_RESOLUTION> {
public:
  /**
   * @brief Convert to grayscale with explicit temperature range.
   * @param[in] t_min Temperature mapped to pixel value 0.
   * @param[in] t_max Temperature mapped to pixel value 255.
   * @return GrayscaleImage with pixels in [0, 255].
   */
  GrayscaleImage toGrayscale(double t_min, double t_max) const;

  /**
   * @brief Convert to grayscale using the automatic min/max of this image.
   * @return GrayscaleImage with pixels in [0, 255].
   */
  GrayscaleImage toGrayscale() const;

  /**
   * @brief Convert to a false-color image (iron palette) with explicit range.
   * @param[in] t_min Temperature mapped to the cold end of the palette.
   * @param[in] t_max Temperature mapped to the hot end of the palette.
   * @return FalseColorImage (RGB, iron palette).
   */
  FalseColorImage toFalseColor(double t_min, double t_max) const;

  /**
   * @brief Convert to a false-color image (iron palette) using automatic min/max.
   * @return FalseColorImage (RGB, iron palette).
   */
  FalseColorImage toFalseColor() const;
};

/**
 * @class  ThermalMeasurement
 * @brief  Structure for holding a measurement from a thermal sensor
 */
struct SENSORRING_EXPORT ThermalMeasurement {

  /// Measurement header
  Header header;

  /// Device health state at the time of publication.
  device::DeviceState state = device::DeviceState::Undefined;

  /// Ambient temperature in °C
  double t_ambient_deg_c = 0;

  /// Minimum temperature recorded in the measurement in °C
  double min_deg_c = 0;

  /// Maximum temperature recorded in the measurement in °C
  double max_deg_c = 0;

  /// Image structure where each pixel represents the temperature measured at that point in °C
  TemperatureImage temperatures;
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart