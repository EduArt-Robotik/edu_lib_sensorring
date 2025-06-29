#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <cmath>
#include "Math.hpp"

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

namespace sensor{

enum class SensorState{
	SensorInit,
	SensorOK,
	ReceiveError
};

}

namespace light{
	enum class LightMode{
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
}

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

struct PointData {
	math::Vector3 point = {0.0, 0.0, 0.0};
	float sigma = 0.0;
	float raw_distance = 0.0;
	int user_idx = 0;
};

// Specific Types used in the Program
using PointCloud        = std::vector<PointData>;
using GrayscaleImage	= GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION>;
using TemperatureImage	= GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
using FalseColorImage	= GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION>;

struct TofMeasurement {
    unsigned int frame_id = 0;
    PointCloud point_cloud;
};

struct ThermalMeasurement {
    unsigned int frame_id = 0;
	unsigned int user_idx = 0;
	double t_ambient_deg_c = 0;
	double min_deg_c = 0;
	double max_deg_c = 0;
    TemperatureImage temp_data_deg_c;
	GrayscaleImage grayscale_img;
	FalseColorImage falsecolor_img;
};

}

}