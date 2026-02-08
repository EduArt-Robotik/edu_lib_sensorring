#pragma once

#include <vector>

#include "hardware/heimann_htpa32.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"

namespace eduart {

namespace device {

struct GetLatestMeasurement {
  struct Request {};
  struct Response {
    const measurement::ThermalMeasurement& measurement;
    SensorState state;
  };
};

struct RequestThermalEeprom {
  struct Request {
    com::ComInterface* interface;
    unsigned int active_sensors;
  };
  struct Response {};
};

struct RequestThermalMeasurement {
  struct Request {
    com::ComInterface* interface;
    unsigned int active_sensors;
  };
  struct Response {};
};

struct FetchThermalMeasurement {
  struct Request {
    com::ComInterface* interface;
    unsigned int active_sensors;
  };
  struct Response {};
};

struct ThermalSensor : BaseDevice, ICapability<GetLatestMeasurement> {
public:
  ThermalSensor(ThermalSensorParams params, com::ComInterface* interface, std::size_t idx);
  ~ThermalSensor();

  void readEEPROM();
  bool gotEEPROM() const;
  bool stopCalibration();
  bool startCalibration(std::size_t window);
  ThermalSensorParams getParams() const;

  std::pair<const measurement::GrayscaleImage&, SensorState> getLatestGrayscaleImage() const;
  std::pair<const measurement::FalseColorImage&, SensorState> getLatestFalseColorImage() const;

  GetLatestMeasurement::Response invoke(const GetLatestMeasurement::Request&) const override;

  static RequestThermalEeprom::Response requestThermalEeprom(const RequestThermalEeprom::Request& req);
  static RequestThermalMeasurement::Response requestThermalMeasurement(const RequestThermalMeasurement::Request& req);
  static FetchThermalMeasurement::Response fetchThermalMeasurement(const FetchThermalMeasurement::Request& req);

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  void rotateLeftImage(measurement::GrayscaleImage& image) const;
  const measurement::FalseColorImage convertToFalseColorImage(const measurement::GrayscaleImage& image) const;
  const measurement::GrayscaleImage convertToGrayscaleImage(const measurement::TemperatureImage& temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const;
  const measurement::ThermalMeasurement processMeasurement(const uint8_t frame_id, const uint8_t* data, const htpa32::HTPA32Eeprom& eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  const ThermalSensorParams _params;
  htpa32::HTPA32Eeprom _eeprom;

  uint16_t _vdd;
  uint16_t _ptat;
  measurement::ThermalMeasurement _latest_measurement;

  uint8_t _rx_buffer[256 * 2 + NUMBER_OF_PIXEL * 2];
  std::size_t _rx_buffer_offset;

  bool _got_eeprom;
  bool _got_calibration;
  bool _calibration_active;
  double _calibration_average;
  std::size_t _calibration_count_current;
  std::size_t _calibration_count_goal;
  std::string _eeprom_filename;
  std::string _calibration_filename;
  measurement::TemperatureImage _calibration_image;
};

} // namespace device

} // namespace eduart