// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   ThermalMeasurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Structures for thermal measurements
 * @date   2025-11-20
 */

#pragma once

#include <cstdint>

#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/Image.hpp"

namespace eduart {

namespace measurement {

/**
 * @class  GrayscaleImage
 * @brief  Grayscale image with 1 channel and 8 bit color depth
 */
class SENSORRING_API GrayscaleImage : public GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION> {};

/**
 * @class  TemperatureImage
 * @brief  Pseudo image structure for the converted temperatures of a thermal image
 */
class SENSORRING_API TemperatureImage : public GenericGrayscaleImage<double, THERMAL_RESOLUTION> {};

/**
 * @class  FalseColorImage
 * @brief  False color image with 3 channels (red, green, blue) and 8 bit color depth
 */
class SENSORRING_API FalseColorImage : public GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION> {};

/**
 * @class  ThermalMeasurement
 * @brief  Structure for holding a measurement form a thermal sensor
 */
struct SENSORRING_API ThermalMeasurement {
  /// Frame number of the ThermalMeasurement
  unsigned int frame_id  = 0;

  /// User assigned index of the sensor that measured the point
  unsigned int user_idx  = 0;

  /// Ambient temperature in °C
  double t_ambient_deg_c = 0;

  /// Minimum temperature recorded in the measurement in °C
  double min_deg_c       = 0;

  /// Maximum temperature recorded in the measurement in °C
  double max_deg_c       = 0;

  /// Image structure where each pixel represents the temperature measured at that point in °C
  TemperatureImage temp_data_deg_c;

  /// Grayscale image visualizing the thermal measurement
  GrayscaleImage grayscale_img;

  /// False color image visualizing the thermal measurement
  FalseColorImage falsecolor_img;
};

} // namespace measurement

} // namespace eduart