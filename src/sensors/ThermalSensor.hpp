#pragma once

#include "BaseSensor.hpp"
#include "CustomTypes.hpp"
#include "Math.hpp"
#include "Parameters.hpp"
#include "interface/ComInterface.hpp"
#include "utils/heimann_htpa32.hpp"

#include <array>
#include <vector>

namespace eduart {

namespace sensor {

class ThermalSensor : public BaseSensor {
public:
  ThermalSensor (ThermalSensorParams params, com::ComInterface *interface, std::size_t idx);
  ~ThermalSensor ();

  void readEEPROM ();
  bool gotEEPROM () const;
  bool stopCalibration ();
  bool startCalibration (size_t window);
  ThermalSensorParams getParams () const;

  std::pair<measurement::GrayscaleImage, SensorState> getLatestGrayscaleImage () const;
  std::pair<measurement::FalseColorImage, SensorState> getLatestFalseColorImage () const;
  std::pair<measurement::ThermalMeasurement, SensorState> getLatestMeasurement () const;

  void canCallback (const com::ComEndpoint source, const std::vector<uint8_t> &data) override;
  void onClearDataFlag () override;

private:
  void rotateLeftImage (measurement::GrayscaleImage &image) const;
  const measurement::FalseColorImage convertToFalseColorImage (const measurement::GrayscaleImage &image) const;
  const measurement::GrayscaleImage convertToGrayscaleImage (const measurement::TemperatureImage &temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const;
  const measurement::ThermalMeasurement processMeasurement (const uint8_t frame_id, const uint8_t *data, const heimannsensor::HTPA32Eeprom &eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const;

  const ThermalSensorParams _params;
  heimannsensor::HTPA32Eeprom _eeprom;

  uint16_t _vdd;
  uint16_t _ptat;
  measurement::ThermalMeasurement _latest_measurement;

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

}

}