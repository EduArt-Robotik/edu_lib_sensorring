#pragma once

#include <vector>
#include <array>
#include "BaseSensor.hpp"
#include "math/MiniMath.hpp"
#include "utils/heimann_htpa32.hpp"
#include "utils/CustomTypes.hpp"

namespace Sensor{

struct ThermalSensorParams {
    int idx = 0;
	double t_min = 0;
	double t_max = 0;
    double fov_x = 0;
    double fov_y = 0;
    unsigned int res_x = 0;
    unsigned int res_y = 0;
	bool auto_min_max = false;
	bool use_eeprom_file = false;
	bool use_calibration_file = false;
	std::string eeprom_dir = "";
	std::string calibration_dir = "";
    mm::Vector3 rotation = {0, 0, 0};
    mm::Vector3 translation = {0, 0, 0};
	SensorOrientation orientation = SensorOrientation::none;
};

class ThermalSensor : public BaseSensor{
    public:
        ThermalSensor(ThermalSensorParams params, std::shared_ptr<edu::SocketCANFD> can_interface, canid_t canid, bool enable);
        ~ThermalSensor();
        
		void readEEPROM();
		bool gotEEPROM() const;
		bool stopCalibration();
		bool startCalibration(size_t window);
		const ThermalSensorParams& getParams() const;

		const Measurement::GrayscaleImage* getLatestGrayscaleImage() const;
		const Measurement::FalseColorImage* getLatestFalseColorImage() const;
		const Measurement::ThermalSensorMeasurement* getLatestMeasurement() const;
		const Measurement::GrayscaleImage* getLatestGrayscaleImage(SensorState &error) const;
		const Measurement::FalseColorImage* getLatestFalseColorImage(SensorState &error) const;
		const Measurement::ThermalSensorMeasurement* getLatestMeasurement(SensorState &error) const;
		

        void canCallback(const canfd_frame& frame) override;
		void onClearDataFlag() override;

    private:
		void rotateLeftImage(Measurement::GrayscaleImage &image) const;
		const Measurement::FalseColorImage convertToFalseColorImage(const Measurement::GrayscaleImage& image) const;
		const Measurement::GrayscaleImage convertToGrayscaleImage(const Measurement::TemperatureImage& temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const;
		const Measurement::ThermalSensorMeasurement processMeasurement(const uint8_t frame_id, const uint8_t* data, const HeimannSensor::HTPA32Eeprom& eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const;

        const ThermalSensorParams _params;
        HeimannSensor::HTPA32Eeprom _eeprom;

		uint16_t _vdd;
		uint16_t _ptat;
		Measurement::GrayscaleImage _latest_grayscale_image;
		Measurement::FalseColorImage _latest_falsecolor_image;
		Measurement::ThermalSensorMeasurement _latest_measurement;

		uint8_t _rx_buffer[256 * 2 + NUMBER_OF_PIXEL * 2];
        unsigned int _rx_buffer_offset;

		bool _got_eeprom;
		bool _got_calibration;
		bool _calibration_active;
		double _calibration_average;
		size_t _calibration_count_current;
		size_t _calibration_count_goal;
		std::string _eeprom_filename;
		std::string _calibration_filename;
		Measurement::TemperatureImage _calibration_image;
};

}; // namespace Sensor