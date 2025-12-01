// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Image.hpp
 * @author EduArt Robotik GmbH
 * @brief  Generic Image types
 * @date   2025-11-20
 */

#pragma once

#include <array>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

static constexpr unsigned int THERMAL_RESOLUTION = 1024;
static constexpr unsigned int MAX_MSG_LENGTH     = 64;

namespace measurement {

/**
 * @class  GenericGrayscaleImage
 * @brief  Template for images with one channel and variable type
 */
template <typename T, std::size_t RESOLUTION> struct SENSORRING_API GenericGrayscaleImage {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

  /// Internal data structure for the image
  std::array<T, RESOLUTION> data = {};

  /**
   * @brief Calculate the average over all pixels
   * @return average
   */
  double avg();

  /**
   * @brief Round each pixel value when a floating point type is used
   * @return Averaged GrayScaleImage of the same type
   */
  GenericGrayscaleImage& round();

  /**
   * @brief Divide each pixel by the same value
   * @return Resulting GrayScaleImage with updated values
   */
  GenericGrayscaleImage& operator/=(const T other);

  /**
   * @brief Add the same value to each pixel
   * @return Resulting GrayScaleImage with updated values
   */
  GenericGrayscaleImage& operator+=(const T other);

  /**
   * @brief Subtract the same value from each pixel
   * @return Resulting GrayScaleImage with updated values
   */
  GenericGrayscaleImage& operator-=(const T other);

  /**
   * @brief Add two GrayScaleImage images pixel wise
   * @return Resulting GrayScaleImage with updated values
   */
  GenericGrayscaleImage& operator+=(const GenericGrayscaleImage& other);

  /**
   * @brief Subtract two GrayScaleImage images pixel wise
   * @return Resulting GrayScaleImage with updated values
   */
  GenericGrayscaleImage& operator-=(const GenericGrayscaleImage& other);

  /**
   * @brief Add two GrayScaleImage images of different data types pixel wise
   * @return Resulting GrayScaleImage with updated values
   */
  template <typename U> GenericGrayscaleImage& operator+=(const GenericGrayscaleImage<U, RESOLUTION>& other);

  /**
   * @brief Subtract two GrayScaleImage images of different data types pixel wise
   * @return Resulting GrayScaleImage with updated values
   */
  template <typename U> GenericGrayscaleImage& operator-=(const GenericGrayscaleImage<U, RESOLUTION>& other);
};

/**
 * @class  GenericRGBImage
 * @brief  Template for images with three channels and variable type
 */
template <typename T, std::size_t RESOLUTION> struct SENSORRING_API GenericRGBImage {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

  /// Internal data structure for the image
  std::array<std::array<T, 3>, RESOLUTION> data = {};
};

} // namespace measurement

} // namespace eduart