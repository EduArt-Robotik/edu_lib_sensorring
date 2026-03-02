#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <string>
#include <vector>

#include "sensorring/device/BaseSensor.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"

#include "HTPA32_Constants.hpp"
#include "HTPA32_Eeprom.hpp"

namespace eduart {

namespace com {
class ComInterface;
} // namespace com

namespace device {

class HTPA32_Device;

/**
 * @class HTPA32_DeviceImpl
 * @brief Implementation class for HTPA32_Device hiding all private members.
 */
class HTPA32_DeviceImpl {
public:
  HTPA32_DeviceImpl(HTPA32_Device& parent, HTPA32_Params params, com::ComInterface* interface, unsigned int idx);
  ~HTPA32_DeviceImpl();

  HTPA32_Params getParams() const;

  std::pair<const measurement::GrayscaleImage&, SensorState> getLatestGrayscaleImage() const;
  std::pair<const measurement::FalseColorImage&, SensorState> getLatestFalseColorImage() const;
  std::pair<const measurement::ThermalMeasurement&, SensorState> getLatestMeasurement() const;

  std::future<bool> getEpromAsync(std::chrono::milliseconds timeout);
  bool stopCalibration();
  bool startCalibration(std::size_t window);

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data);

  void onResetSensorState();
  void onClearDataFlag();

private:
  void rotateLeftImage(measurement::GrayscaleImage& image) const;
  measurement::FalseColorImage convertToFalseColorImage(const measurement::GrayscaleImage& image) const;
  measurement::GrayscaleImage convertToGrayscaleImage(const measurement::TemperatureImage& temp_data_deg_c, double t_min_deg_c, double t_max_deg_c) const;
  measurement::ThermalMeasurement processMeasurement(uint8_t frame_id, const uint8_t* data, const htpa32::HTPA32_Eeprom& eeprom, uint16_t vdd, uint16_t ptat, std::size_t len) const;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  HTPA32_Device& _parent;

  const HTPA32_Params _params;
  htpa32::HTPA32_Eeprom _eeprom{};
  std::vector<uint8_t> _eeprom_buffer;

  uint16_t _vdd  = 0;
  uint16_t _ptat = 0;
  measurement::ThermalMeasurement _latest_measurement;

  uint8_t _rx_buffer[256 * 2 + NUMBER_OF_PIXEL * 2]{};
  std::size_t _rx_buffer_offset = 0;

  std::atomic<bool> _read_eeprom{ false };
  std::atomic<bool> _got_eeprom{ false };
  bool _got_calibration                  = false;
  bool _calibration_active               = false;
  double _calibration_average            = 0.0;
  std::size_t _calibration_count_current = 0;
  std::size_t _calibration_count_goal    = 0;
  std::string _eeprom_filename;
  std::string _calibration_filename;
  measurement::TemperatureImage _calibration_image;
  mutable std::condition_variable _eeprom_condition;
  bool _has_ready_measurement = false;
};

} // namespace device

} // namespace eduart
