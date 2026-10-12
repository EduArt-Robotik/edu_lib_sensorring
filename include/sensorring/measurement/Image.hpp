// Copyright (c) 2026 EduArt Robotik GmbH

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

namespace sensorring {

/// Thermal image resolution constant (32x32).
static constexpr unsigned int THERMAL_RESOLUTION = 1024;

/// Maximum message length in bytes.
static constexpr unsigned int MAX_MSG_LENGTH = 64;

namespace measurement {

/**
 * @class  ScalarImage
 * @brief  Template for images with one scalar value per pixel and variable arithmetic type
 */
template <typename T, std::size_t RESOLUTION> struct SENSORRING_EXPORT ScalarImage {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

  /// Internal data structure for the image
  std::array<T, RESOLUTION> data = {};

  /**
   * @brief Copy image data to a pre-allocated buffer
   * @param[out] buffer Pointer to the buffer. Make sure it has sufficient size.
   * @param[in] size Actual size of the buffer (number of elements). If smaller than
   *                 RESOLUTION, only a subset of pixels is copied.
   */
  void copyTo(T* buffer, int size);

  /**
   * @brief Calculate the average over all pixels
   * @return average
   */
  double avg();

  /**
   * @brief Round each pixel value when a floating point type is used
   * @return ScalarImage of the same type with rounded values
   */
  ScalarImage& round();

  /**
   * @brief Divide each pixel by the same value
   * @return Resulting ScalarImage with updated values
   */
  ScalarImage& operator/=(const T other);

  /**
   * @brief Add the same value to each pixel
   * @return Resulting ScalarImage with updated values
   */
  ScalarImage& operator+=(const T other);

  /**
   * @brief Subtract the same value from each pixel
   * @return Resulting ScalarImage with updated values
   */
  ScalarImage& operator-=(const T other);

  /**
   * @brief Add two ScalarImage images pixel wise
   * @return Resulting ScalarImage with updated values
   */
  ScalarImage& operator+=(const ScalarImage& other);

  /**
   * @brief Subtract two ScalarImage images pixel wise
   * @return Resulting ScalarImage with updated values
   */
  ScalarImage& operator-=(const ScalarImage& other);

  /**
   * @brief Add two ScalarImage images of different data types pixel wise
   * @return Resulting ScalarImage with updated values
   */
  template <typename U> ScalarImage& operator+=(const ScalarImage<U, RESOLUTION>& other);

  /**
   * @brief Subtract two ScalarImage images of different data types pixel wise
   * @return Resulting ScalarImage with updated values
   */
  template <typename U> ScalarImage& operator-=(const ScalarImage<U, RESOLUTION>& other);
};

/**
 * @class  RgbImage
 * @brief  Template for images with three channels (R, G, B) and variable arithmetic type
 */
template <typename T, std::size_t RESOLUTION> struct SENSORRING_EXPORT RgbImage {
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

  /// Internal data structure for the image
  std::array<std::array<T, 3>, RESOLUTION> data = {};

  /**
   * @brief Copy image data to a pre-allocated buffer (flattened: R,G,B,R,G,B,...)
   * @param[out] buffer Pointer to the buffer. Make sure it has sufficient size.
   * @param[in] size Actual size of the buffer (number of elements). If smaller than
   *                 RESOLUTION*3, only a subset of pixels is copied.
   */
  void copyTo(T* buffer, int size);
};

} // namespace measurement

} // namespace sensorring

} // namespace eduart