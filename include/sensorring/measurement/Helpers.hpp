// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Helpers.hpp
 * @author EduArt Robotik GmbH
 * @brief  Free helper functions for combining point clouds and converting thermal images.
 */

#pragma once

#include <vector>

#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/measurement/PointCloud.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

/**
 * @brief Combine multiple point clouds into one.
 * @param[in] clouds Vector of point clouds to merge.
 * @return Single PointCloud containing all points.
 */
SENSORRING_EXPORT PointCloud combinePointClouds(const std::vector<PointCloud>& clouds);

/**
 * @brief Combine point clouds from multiple depth measurements into one.
 * @param[in] measurements Vector of depth measurements whose point clouds will be merged.
 * @return Single PointCloud containing all points (uses transformed_point_cloud).
 */
SENSORRING_EXPORT PointCloud combinePointClouds(const std::vector<DepthMeasurement>& measurements);

/**
 * @brief Convert a temperature image to grayscale with explicit temperature range.
 * @param[in] img Temperature image (°C per pixel).
 * @param[in] t_min Minimum temperature for mapping (maps to 0).
 * @param[in] t_max Maximum temperature for mapping (maps to 255).
 * @return GrayscaleImage with pixels in [0, 255].
 */
SENSORRING_EXPORT GrayscaleImage toGrayscale(const TemperatureImage& img, double t_min, double t_max);

/**
 * @brief Convert a temperature image to grayscale using automatic min/max from the image data.
 * @param[in] img Temperature image (°C per pixel).
 * @return GrayscaleImage with pixels in [0, 255].
 */
SENSORRING_EXPORT GrayscaleImage toGrayscale(const TemperatureImage& img);

/**
 * @brief Convert a temperature image to a false-color (iron palette) image with explicit range.
 * @param[in] img Temperature image (°C per pixel).
 * @param[in] t_min Minimum temperature for mapping.
 * @param[in] t_max Maximum temperature for mapping.
 * @return FalseColorImage (RGB, iron palette).
 */
SENSORRING_EXPORT FalseColorImage toFalseColor(const TemperatureImage& img, double t_min, double t_max);

/**
 * @brief Convert a temperature image to a false-color (iron palette) image using automatic min/max.
 * @param[in] img Temperature image (°C per pixel).
 * @return FalseColorImage (RGB, iron palette).
 */
SENSORRING_EXPORT FalseColorImage toFalseColor(const TemperatureImage& img);

} // namespace measurement

} // namespace sensorring

} // namespace eduart
