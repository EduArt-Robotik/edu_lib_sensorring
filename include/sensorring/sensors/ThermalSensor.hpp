#pragma once

#include <vector>
#include <array>
#include "ComInterface.hpp"
#include "BaseSensor.hpp"
#include "Math.hpp"
#include "heimann_htpa32.hpp"
#include "CustomTypes.hpp"
#include "Parameters.hpp"

namespace sensor{

class ThermalSensor : public BaseSensor{
    public:
        ThermalSensor(ThermalSensorParams params, com::ComInterface* interface, bool enable);
        ~ThermalSensor();
        
		void readEEPROM();
		bool gotEEPROM() const;
		bool stopCalibration();
		bool startCalibration(size_t window);
		const ThermalSensorParams& getParams() const;

		const measurement::GrayscaleImage* getLatestGrayscaleImage() const;
		const measurement::FalseColorImage* getLatestFalseColorImage() const;
		const measurement::ThermalSensorMeasurement* getLatestMeasurement() const;
		const measurement::GrayscaleImage* getLatestGrayscaleImage(SensorState &error) const;
		const measurement::FalseColorImage* getLatestFalseColorImage(SensorState &error) const;
		const measurement::ThermalSensorMeasurement* getLatestMeasurement(SensorState &error) const;
		

        void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;
		void onClearDataFlag() override;

    private:
		void rotateLeftImage(measurement::GrayscaleImage &image) const;
		const measurement::FalseColorImage convertToFalseColorImage(const measurement::GrayscaleImage& image) const;
		const measurement::GrayscaleImage convertToGrayscaleImage(const measurement::TemperatureImage& temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const;
		const measurement::ThermalSensorMeasurement processMeasurement(const uint8_t frame_id, const uint8_t* data, const heimannsensor::HTPA32Eeprom& eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const;

        const ThermalSensorParams _params;
        heimannsensor::HTPA32Eeprom _eeprom;

		uint16_t _vdd;
		uint16_t _ptat;
		measurement::GrayscaleImage _latest_grayscale_image;
		measurement::FalseColorImage _latest_falsecolor_image;
		measurement::ThermalSensorMeasurement _latest_measurement;

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
		measurement::TemperatureImage _calibration_image;
};

}; // namespace Sensor