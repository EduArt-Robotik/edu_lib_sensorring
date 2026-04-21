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

#include "sensorring/device/DeviceState.hpp"
#include "sensorring/measurement/Image.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

/**
 * @class  GrayscaleImage
 * @brief  Grayscale image with 1 channel and 8 bit color depth
 */
class GrayscaleImage : public GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION> {};

/**
 * @class  TemperatureImage
 * @brief  Pseudo image structure for the converted temperatures of a thermal image
 */
class TemperatureImage : public GenericGrayscaleImage<double, THERMAL_RESOLUTION> {};

/**
 * @class  FalseColorImage
 * @brief  False color image with 3 channels (red, green, blue) and 8 bit color depth
 */
class FalseColorImage : public GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION> {};

/**
 * @class  ThermalMeasurement
 * @brief  Structure for holding a measurement from a thermal sensor
 */
struct ThermalMeasurement {
  /// Index of the sensor that produced this measurement.
  unsigned int sensor_index = 0;

  /// Frame number of the ThermalMeasurement
  unsigned int frame_id = 0;

  /// Timestamp when the measurement was taken.
  std::chrono::system_clock::time_point timestamp;

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

  /// Grayscale image visualizing the thermal measurement
  GrayscaleImage grayscale_img;

  /// False color image visualizing the thermal measurement
  FalseColorImage falsecolor_img;
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart