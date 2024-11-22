#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <cmath>

#define THERMAL_RESOLUTION 1024
#define MAX_MSG_LENGTH 64

namespace math{

struct Vector3 {
	std::array<double, 3> data;

	double& x();
	const double& x() const;
	double& y();
	const double& y() const;
	double& z();
	const double& z() const;
	double& operator[](std::size_t idx);
	const double& operator[](std::size_t idx) const;
};

};

namespace measurement{

template<typename T, std::size_t RESOLUTION>
struct GenericGrayscaleImage {
	std::array<T, RESOLUTION> data = {};
};

template<typename T, std::size_t RESOLUTION>
struct GenericRGBImage {
	std::array<std::array<T, 3>, RESOLUTION> data = {};
};

// Specific Types used in the Program
using PointCloud        = std::vector<math::Vector3>;
using GrayscaleImage	= GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION>;
using TemperatureImage	= GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
using FalseColorImage	= GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION>;

struct TofMeasurement {
	int frame_id = 0;
	unsigned int size = 0;
	std::vector<float> point_distance;
	std::vector<float> point_sigma;
	PointCloud point_data;
	PointCloud point_data_transformed;

	void reserve(std::size_t size);
};

struct ThermalMeasurement {
	int frame_id = 0;
	double t_ambient_deg_c = 0;
	double min_deg_c = 0;
	double max_deg_c = 0;
	TemperatureImage temp_data_deg_c;
	GrayscaleImage grayscale_img;
	FalseColorImage falsecolor_img;
};

};