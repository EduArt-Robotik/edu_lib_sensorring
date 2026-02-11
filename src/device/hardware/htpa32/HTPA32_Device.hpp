#pragma once

#include <chrono>
#include <condition_variable>
#include <future>
#include <vector>

#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"

#include "heimann_htpa32.hpp"

namespace eduart {

namespace device {

struct HTPA32_Device : BaseDevice {
public:
  HTPA32_Device(HTPA32_Params params, com::ComInterface* interface, unsigned int idx);
  ~HTPA32_Device();

  HTPA32_Params getParams() const;

  std::pair<const measurement::GrayscaleImage&, SensorState> getLatestGrayscaleImage() const;
  std::pair<const measurement::FalseColorImage&, SensorState> getLatestFalseColorImage() const;

  // Explicit methods replacing capability-based invoke APIs.
  std::future<bool> getEpromAsync(std::chrono::milliseconds timeout);
  bool stopCalibration();
  bool startCalibration(std::size_t window);
  std::pair<const measurement::ThermalMeasurement&, SensorState> getLatestMeasurement() const;
  std::future<bool> requestThermalMeasurementAsync(std::chrono::milliseconds timeout);
  std::future<bool> fetchThermalMeasurementAsync(std::chrono::milliseconds timeout);

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  void rotateLeftImage(measurement::GrayscaleImage& image) const;
  const measurement::FalseColorImage convertToFalseColorImage(const measurement::GrayscaleImage& image) const;
  const measurement::GrayscaleImage convertToGrayscaleImage(const measurement::TemperatureImage& temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const;
  const measurement::ThermalMeasurement processMeasurement(const uint8_t frame_id, const uint8_t* data, const htpa32::HTPA32Eeprom& eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  const HTPA32_Params _params;
  htpa32::HTPA32Eeprom _eeprom;

  uint16_t _vdd;
  uint16_t _ptat;
  measurement::ThermalMeasurement _latest_measurement;

  uint8_t _rx_buffer[256 * 2 + NUMBER_OF_PIXEL * 2];
  std::size_t _rx_buffer_offset;

  std::atomic<bool> _got_eeprom;
  bool _got_calibration;
  bool _calibration_active;
  double _calibration_average;
  std::size_t _calibration_count_current;
  std::size_t _calibration_count_goal;
  std::string _eeprom_filename;
  std::string _calibration_filename;
  measurement::TemperatureImage _calibration_image;
  mutable std::condition_variable _eeprom_condition;
  mutable std::condition_variable _data_condition;
};

} // namespace device

} // namespace eduart
