#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <cmath>

#define THERMAL_RESOLUTION 1024

// Forward declaration
namespace mm{
	struct Vector3;
}

namespace HeimannSensor{

// Needs to be packed because µC memory is directly copied in this struct
struct __attribute__((__packed__)) HTPA32Eeprom{
	float		pixc_min;
	float		pixc_max;
	uint8_t		grad_scale;
	uint16_t	tablenumber;
	uint8_t		epsilon;
	uint8_t		mbit_calib;
	uint8_t		bias_calib;
	uint8_t		clk_calib;
	uint8_t		bpa_calib;
	uint8_t		pu_calib;
	uint8_t		arraytype;
	uint16_t	vddth1;
	uint16_t	vddth2;
	float		ptat_gradient;
	float		ptat_offset;
	uint16_t	ptat_th1;
	uint16_t	ptat_th2;
	uint8_t		vddsc_gradient;
	uint8_t		vddsc_offset;
	uint8_t		global_offset;
	uint16_t	global_gain;
	uint8_t		mbit_user;
	uint8_t		bias_user;
	uint8_t		clk_user;
	uint8_t		bpa_user;
	uint8_t		pu_user;
	uint32_t	device_id;
	uint8_t		norof_deadpix;
	uint16_t	deadpix_addr[24];
	uint16_t	deadpix_mask[12];
	int16_t		vddcomp_gradient[256];
	int16_t		vddcomp_offset[256];
	int16_t		th_gradient[1024];
	int16_t		th_offset[1024];
	uint16_t	p[1024];
};

}; //namespace HeimannSensor


namespace Measurement{

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

using PointCloud        = std::vector<mm::Vector3>;
using GrayscaleImage	= GenericGrayscaleImage<uint8_t, THERMAL_RESOLUTION>;
using TemperatureImage	= GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
using FalseColorImage	= GenericRGBImage<uint8_t, THERMAL_RESOLUTION>;


struct TofSensorMeasurement {
    int frame_id = 0;
    unsigned int length = 0;
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
};

}; //namespace Measurement