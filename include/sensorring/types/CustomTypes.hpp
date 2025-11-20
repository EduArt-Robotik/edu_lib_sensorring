// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   CustomTypes.hpp
 * @author EduArt Robotik GmbH
 * @brief  Custom types used by the sensorring library
 * @date   2024-11-06
 */

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "sensorring/platform/SensorringExport.hpp"

#include "Math.hpp"

namespace eduart {

static constexpr unsigned int THERMAL_RESOLUTION = 1024;
static constexpr unsigned int MAX_MSG_LENGTH     = 64;

namespace com {

enum class SENSORRING_API InterfaceType {
  UNDEFINED,
  SOCKETCAN,
  USBTINGO
};

} // namespace com

namespace sensor {

enum class SENSORRING_API SensorState {
  SensorInit,
  SensorOK,
  ReceiveError
};

} // namespace sensor

namespace light {
enum class SENSORRING_API LightMode {
  Off,
  Dimmed,
  HighBeam,
  FlashAll,
  FlashLeft,
  FlashRight,
  Pulsation,
  Rotation,
  Running,
  MapDistance
};
} // namespace light

namespace measurement {

template <typename T, std::size_t RESOLUTION> struct SENSORRING_API GenericGrayscaleImage {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");
  std::array<T, RESOLUTION> data = {};

  double avg();
  GenericGrayscaleImage& round();
  GenericGrayscaleImage& operator/=(const T other);
  GenericGrayscaleImage& operator+=(const T other);
  GenericGrayscaleImage& operator-=(const T other);
  GenericGrayscaleImage& operator+=(const GenericGrayscaleImage& other);
  GenericGrayscaleImage& operator-=(const GenericGrayscaleImage& other);
  template <typename U> GenericGrayscaleImage& operator+=(const GenericGrayscaleImage<U, RESOLUTION>& other);
  template <typename U> GenericGrayscaleImage& operator-=(const GenericGrayscaleImage<U, RESOLUTION>& other);
};

template <typename T, std::size_t RESOLUTION> struct SENSORRING_API GenericRGBImage {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");
  std::array<std::array<T, 3>, RESOLUTION> data = {};
};

struct SENSORRING_API PointData {
  math::Vector3 point = { 0.0, 0.0, 0.0 };
  double raw_distance = 0.0;
  double sigma        = 0.0;
  int user_idx        = 0;
};

// Specific Types used in the Program
using PointCloud       = std::vector<PointData>;
using GrayscaleImage   = GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION>;
using TemperatureImage = GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
using FalseColorImage  = GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION>;

SENSORRING_API void copyPointCloudTo(const PointCloud& pc, double* buffer, int size);

struct SENSORRING_API TofMeasurement {
  unsigned int frame_id = 0;
  PointCloud point_cloud;
};

struct SENSORRING_API ThermalMeasurement {
  unsigned int frame_id  = 0;
  unsigned int user_idx  = 0;
  double t_ambient_deg_c = 0;
  double min_deg_c       = 0;
  double max_deg_c       = 0;
  TemperatureImage temp_data_deg_c;
  GrayscaleImage grayscale_img;
  FalseColorImage falsecolor_img;
};

} // namespace measurement

} // namespace eduart