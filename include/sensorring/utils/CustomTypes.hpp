#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <cmath>
#include "Math.hpp"

#define THERMAL_RESOLUTION 1024
#define MAX_MSG_LENGTH 64

namespace sensor{
	enum class SensorOrientation{
		left,
		right,
		none
	};

	enum class SensorState{
		SensorOK,
		ReceiveError
	};
} // namespace sensor

namespace measurement{

template<typename T, std::size_t RESOLUTION>
struct GenericGrayscaleImage {
	static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");
	std::array<T, RESOLUTION> data = {};

	double avg ();
	GenericGrayscaleImage& round ();
	GenericGrayscaleImage& operator/= (const T other);
	GenericGrayscaleImage& operator+= (const T other);
	GenericGrayscaleImage& operator-= (const T other);
	GenericGrayscaleImage& operator+= (const GenericGrayscaleImage& other);
	GenericGrayscaleImage& operator-= (const GenericGrayscaleImage& other);
	template <typename U>
	GenericGrayscaleImage& operator+= (const GenericGrayscaleImage<U, RESOLUTION>& other);
	template <typename U>
	GenericGrayscaleImage& operator-= (const GenericGrayscaleImage<U, RESOLUTION>& other);
};

template<typename T, std::size_t RESOLUTION>
struct GenericRGBImage {
	static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");
	std::array<std::array<T, 3>, RESOLUTION> data = {};
};

// Specific Types used in the Program
using PointCloud        = std::vector<math::Vector3>;
using GrayscaleImage	= GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION>;
using TemperatureImage	= GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
using FalseColorImage	= GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION>;

struct TofSensorMeasurement {
    int frame_id = 0;
    unsigned int size = 0;
    std::vector<float> point_distance;
    std::vector<float> point_sigma;
    PointCloud point_data;
    PointCloud point_data_transformed;

    void reserve(std::size_t size);
};

struct ThermalSensorMeasurement {
    int frame_id = 0;
	double t_ambient_deg_c = 0;
	double min_deg_c = 0;
	double max_deg_c = 0;
    TemperatureImage temp_data_deg_c;
	GrayscaleImage grayscale_img;
	FalseColorImage falsecolor_img;
};

}; //namespace measurement