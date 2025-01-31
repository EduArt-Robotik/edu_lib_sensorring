#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <cmath>

#define THERMAL_RESOLUTION 1024
#define MAX_MSG_LENGTH 64

namespace eduart{

namespace com{

enum class DeviceType{
    UNDEFINED,
    SOCKETCAN,
    USBTINGO
};

}

namespace math{

/**
 * @enum Vector3
 * @brief Lightweight implementation of a three dimensional vector of type double.
 */
struct Vector3 {
	std::array<double, 3> data;

	/**
	 * Index the vector.
	 * @return Returns the first element of the vector.
	 */
	double& x();
	const double& x() const;

	/**
	 * Index the vector.
	 * @return Returns the second element of the vector.
	 */
	double& y();
	const double& y() const;

	/**
	 * Index the vector.
	 * @return Returns the third element of the vector.
	 */
	double& z();
	const double& z() const;

	/**
	 * Index the vector.
	 * @param[in] idx Index of the Vector. Valid values are [0, 1, 2].
	 * @return Returns element of the vector at the specified index.
	 */
	double& operator[](std::size_t idx);
	const double& operator[](std::size_t idx) const;
};

}

namespace measurement{

/**
 * @enum GenericGrayscaleImage
 * @brief Generic structure for images with one (1) channel. Typically used for grayscale images.
 */
template<typename T, std::size_t RESOLUTION>
struct GenericGrayscaleImage {
	std::array<T, RESOLUTION> data = {};
};

/**
 * @enum GenericRGBImage
 * @brief Generic structure for images with three (3) channels. Typically used for rgb images.
 */
template<typename T, std::size_t RESOLUTION>
struct GenericRGBImage {
	std::array<std::array<T, 3>, RESOLUTION> data = {};
};

// Specific Types used for the measurements
using PointCloud        = std::vector<math::Vector3>;
using GrayscaleImage	= GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION>;
using TemperatureImage	= GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
using FalseColorImage	= GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION>;

/**
 * @enum TofMeasurement
 * @brief Structure to hold the measurement from a single Time-of-Flight sensor or a combined measurment from multiple sensors.
 */
struct TofMeasurement {
	int frame_id = 0;
	unsigned int size = 0;
	std::vector<float> point_distance;
	std::vector<float> point_sigma;
	PointCloud point_data;
	PointCloud point_data_transformed;
};

/**
 * @enum ThermalMeasurement
 * @brief Structure to hold the measurement from a single thermal sensor or.
 */
struct ThermalMeasurement {
	int frame_id = 0;
	double t_ambient_deg_c = 0;
	double min_deg_c = 0;
	double max_deg_c = 0;
	TemperatureImage temp_data_deg_c;
	GrayscaleImage grayscale_img;
	FalseColorImage falsecolor_img;
};

}

}