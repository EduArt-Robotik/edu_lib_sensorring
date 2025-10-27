#include "ThermalSensor.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "interface/ComInterface.hpp"
#include "interface/can/canprotocol.hpp"
#include "logger/Logger.hpp"
#include "utils/FileManager.hpp"
#include "utils/Iron.hpp"

namespace eduart {

namespace sensor {

ThermalSensor::ThermalSensor(ThermalSensorParams params, com::ComInterface* interface, std::size_t idx)
    : BaseSensor(interface, com::ComEndpoint("thermal" + std::to_string(idx) + "_data"), idx, params.enable)
    , _params(params) {

  _rx_buffer_offset = 0;
  _interface->addThermalSensorToEndpointMap(idx);
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);

  _vdd  = 0;
  _ptat = 0;

  _got_eeprom                = false;
  _got_calibration           = false;
  _calibration_active        = false;
  _calibration_count_current = 0;
  _calibration_count_goal    = 0;
  _calibration_average       = 0;

  _eeprom_filename      = "sensor" + std::to_string(_idx) + "_hpta32_eeprom.bin";
  _calibration_filename = "sensor" + std::to_string(_idx) + "_hpta32_calibration.txt";

  if (_params.use_eeprom_file) {
    _got_eeprom = filemanager::StructHandler<htpa32::HTPA32Eeprom>::readStructFromFile(_params.eeprom_dir, _eeprom_filename, _eeprom);
  }

  if (_params.use_calibration_file) {
    _got_calibration     = filemanager::ArrayHandler<double, NUMBER_OF_PIXEL>::readArrayFromFile(_params.calibration_dir, _calibration_filename, _calibration_image.data);
    _calibration_average = _calibration_image.avg();
  }
}

ThermalSensor::~ThermalSensor() {
}

ThermalSensorParams ThermalSensor::getParams() const {
  return _params;
}

std::pair<measurement::GrayscaleImage, SensorState> ThermalSensor::getLatestGrayscaleImage() const {
  return { _latest_measurement.grayscale_img, _error };
}

std::pair<measurement::FalseColorImage, SensorState> ThermalSensor::getLatestFalseColorImage() const {
  return { _latest_measurement.falsecolor_img, _error };
}

std::pair<measurement::ThermalMeasurement, SensorState> ThermalSensor::getLatestMeasurement() const {
  return { _latest_measurement, _error };
}

bool ThermalSensor::gotEEPROM() const {
  return _got_eeprom;
}

void ThermalSensor::readEEPROM() {
  if (!_got_eeprom) {
    _got_eeprom       = false;
    _rx_buffer_offset = 0;
  }
}

bool ThermalSensor::stopCalibration() {
  bool result         = _calibration_active;
  _calibration_active = false;
  return result;
}

bool ThermalSensor::startCalibration(size_t window) {
  if (!_calibration_active) {
    _calibration_active        = true;
    _calibration_count_goal    = window;
    _calibration_count_current = 0;
    return true;
  } else {
    return false;
  }
}

void ThermalSensor::onClearDataFlag() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset = 0;
}

void ThermalSensor::canCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  std::size_t msg_size = data.size();

  if (!_got_eeprom) {

    // check if there is still data to be written
    if ((_rx_buffer_offset + msg_size) < (int)sizeof(htpa32::HTPA32Eeprom) + MAX_MSG_LENGTH) {

      // have to account fot last few values of the eeprom because sizeof(ETPA32Eeprom) is not dividable by 64 ( = canfd
      // msg length)
      std::size_t len = (int)sizeof(htpa32::HTPA32Eeprom) - _rx_buffer_offset;
      if (len > msg_size)
        len = msg_size;

      std::copy_n(data.begin(), len, (uint8_t*)&_eeprom + _rx_buffer_offset);
      _rx_buffer_offset += len;

      if (_rx_buffer_offset >= (int)sizeof(htpa32::HTPA32Eeprom)) {
        _got_eeprom = true;
        filemanager::StructHandler<htpa32::HTPA32Eeprom>::saveStructToFile(_params.eeprom_dir, _eeprom_filename, _eeprom);
      }
    }

  } else {
    if (!_new_measurement_ready_flag) {
      // vdd and ptat  message
      if (msg_size == 4) {
        _vdd  = (uint16_t)(data[0] << 0 | data[1] << 8);
        _ptat = (uint16_t)(data[2] << 0 | data[3] << 8);

        // data message
      } else if (msg_size == MAX_MSG_LENGTH) {
        if ((_rx_buffer_offset + msg_size) <= (int)sizeof(_rx_buffer)) {
          std::copy_n(data.begin(), msg_size, (uint8_t*)&_rx_buffer + _rx_buffer_offset);
          _rx_buffer_offset += msg_size;

          if (_rx_buffer_offset >= sizeof(_rx_buffer)) {
            _latest_measurement = processMeasurement(0, _rx_buffer, _eeprom, _vdd, _ptat, NUMBER_OF_PIXEL);

            // calibration routine
            if (_calibration_active) {
              if (_calibration_count_current < _calibration_count_goal) {
                _calibration_image += _latest_measurement.temp_data_deg_c;
                _calibration_count_current++;
              }
              if (_calibration_count_current >= _calibration_count_goal) {
                _calibration_image /= _calibration_count_current;
                _calibration_average = _calibration_image.avg();
                _calibration_active  = false;
                _got_calibration     = true;

                if (_params.use_calibration_file) {
                  filemanager::ArrayHandler<double, NUMBER_OF_PIXEL>::saveArrayToFile(_params.calibration_dir, _calibration_filename, _calibration_image.data);
                }
              }
            }

            // apply calibration
            if (!_calibration_active && _got_calibration) {
              _latest_measurement.temp_data_deg_c -= _calibration_image;
              _latest_measurement.temp_data_deg_c += _calibration_average;
            }

            if (_params.auto_min_max) {
              _latest_measurement.grayscale_img = convertToGrayscaleImage(_latest_measurement.temp_data_deg_c, _latest_measurement.min_deg_c, _latest_measurement.max_deg_c);
            } else {
              _latest_measurement.grayscale_img = convertToGrayscaleImage(_latest_measurement.temp_data_deg_c, _params.t_min_deg_c, _params.t_max_deg_c);
            }

            rotateLeftImage(_latest_measurement.grayscale_img);
            _latest_measurement.falsecolor_img = convertToFalseColorImage(_latest_measurement.grayscale_img);
            _new_measurement_ready_flag        = true;
          }
        } else {
          _error = SensorState::ReceiveError;
        }
      }
    }
  }
}

const measurement::ThermalMeasurement ThermalSensor::processMeasurement(const uint8_t frame_id, const uint8_t* data, const htpa32::HTPA32Eeprom& eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const {
  uint16_t* offset_data    = (uint16_t*)(data + 0);   //  256 bytes of buffer are top offset values
  uint16_t* raw_pixel_data = (uint16_t*)(data + 512); // 2048 bytes of buffer are pixel values

  std::vector<double> buffer(len);

  measurement::ThermalMeasurement result;
  result.user_idx  = _params.user_idx;
  result.frame_id  = frame_id;
  result.min_deg_c = 1e6;

  // ambient temperature
  float t_ambient        = _ptat * eeprom.ptat_gradient + eeprom.ptat_offset;
  result.t_ambient_deg_c = (t_ambient - 2732) / 10.0F;

  // process raw buffer to thermal image
  for (unsigned int i = 0; i < len; i++) {
    int idx = i % 128;
    if (i >= (len / 2))
      idx += 128;

    // deserialize thermal measurements. Everything is little endian except the raw pixel values which are big endian.
    uint16_t raw_pixel = (((uint8_t*)raw_pixel_data)[i * 2 + 0] << 8) | (((uint8_t*)raw_pixel_data)[i * 2 + 1] << 0);

    // thermal offsets
    buffer[i] = raw_pixel - ((double)(eeprom.th_gradient[i] * ptat) / pow(2, eeprom.grad_scale)) - eeprom.th_offset[i];

    // electrical offsets
    buffer[i] -= offset_data[idx];

    // The buffer can be used as an image from here on (after subtracting the electric offsets). The following steps are
    // only to calculate temperatures in °C.

    // vdd compensation
    double vdd_comp_val1 = ((double)(_eeprom.vddcomp_gradient[idx] * ptat) / std::pow(2, _eeprom.vddsc_gradient) + _eeprom.vddcomp_offset[idx]) / std::pow(2, _eeprom.vddsc_offset);
    double vdd_comp_val2 = (vdd - _eeprom.vddth1 - ((double)(_eeprom.vddth2 - _eeprom.vddth1) / (_eeprom.ptat_th2 - _eeprom.ptat_th1)) * (ptat - _eeprom.ptat_th1));
    buffer[i]            = buffer[i] - (vdd_comp_val1 * vdd_comp_val2);

    // look-up table and bilinear interpolation
    unsigned int table_col = 0;
    for (int j = 0; j < NROFTAELEMENTS; j++) {
      if (t_ambient > htpa32::XTATemps[j]) {
        table_col = j;
      }
    }

    unsigned int table_row = buffer[i] + TABLEOFFSET;
    table_row              = table_row >> ADEXPBITS; // ToDo: Table row too large. Causes Segfault when accessing the TempTable

    if ((table_row < NROFADELEMENTS) && (table_col < NROFTAELEMENTS)) {
      int dta = t_ambient - htpa32::XTATemps[table_col];

      double vx = ((((int32_t)htpa32::TempTable[table_row][table_col + 1] - (int32_t)htpa32::TempTable[table_row][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)htpa32::TempTable[table_row][table_col];
      double vy = ((((int32_t)htpa32::TempTable[table_row + 1][table_col + 1] - (int32_t)htpa32::TempTable[table_row + 1][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)htpa32::TempTable[table_row + 1][table_col];
      buffer[i] = (uint32_t)((vy - vx) * ((int32_t)(buffer[i] + TABLEOFFSET) - (int32_t)htpa32::YADValues[table_row]) / (int32_t)ADEQUIDISTANCE + (int32_t)vx);

      // apply global offset
      // buffer[i] += _eeprom.global_offset;

      // calculate temperature in °C

      result.temp_data_deg_c.data[i] = (buffer[i] - 2732.0F) / 10.0F;
      if (result.temp_data_deg_c.data[i] < result.min_deg_c)
        result.min_deg_c = result.temp_data_deg_c.data[i];
      if (result.temp_data_deg_c.data[i] > result.max_deg_c)
        result.max_deg_c = result.temp_data_deg_c.data[i];
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Error occurred while processing thermal image");
    }
  }

  return result;
}

const measurement::GrayscaleImage ThermalSensor::convertToGrayscaleImage(const measurement::TemperatureImage& temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const {
  measurement::GrayscaleImage result;

  // pixel data with min - max scaling
  double delta_t = (t_max_deg_c - t_min_deg_c);
  if (delta_t != 0) {
    for (unsigned int i = 0; i < 512; i++) {
      double norm_val = ((temp_data_deg_c.data[i] - t_min_deg_c) / delta_t) * 255;
      result.data[i]  = static_cast<uint8_t>(std::clamp(std::round(norm_val), 0.0, 255.0));
    }

    // rearrange lower half of the image. See Heimann HTPA32 Datasheet.
    for (unsigned int i = 512; i < 1024; i += 128) {
      for (unsigned int j = 0; j < 128; j += 32) {
        for (unsigned int k = 0; k < 32; k++) {
          double norm_val        = (temp_data_deg_c.data[i + (96 - j) + k] - t_min_deg_c) / delta_t * 255;
          result.data[i + j + k] = static_cast<uint8_t>(std::clamp(std::round(norm_val), 0.0, 255.0));
        }
      }
    }
  }

  return result;
}

const measurement::FalseColorImage ThermalSensor::convertToFalseColorImage(const measurement::GrayscaleImage& image) const {
  measurement::FalseColorImage color_image;

  // convert latest measurement to false color image
  for (size_t i = 0; i < NUMBER_OF_PIXEL; i++) {
    color_image.data[i][0] = ThermalPalette::Iron[image.data[i]][0];
    color_image.data[i][1] = ThermalPalette::Iron[image.data[i]][1];
    color_image.data[i][2] = ThermalPalette::Iron[image.data[i]][2];
  }

  return color_image;
}

void ThermalSensor::rotateLeftImage(measurement::GrayscaleImage& image) const {
  if (_params.orientation == sensor::Orientation::left) {
    std::reverse(image.data.begin(), image.data.end());
  }
}

void ThermalSensor::cmdRequestEEPROM(com::ComInterface* interface, std::uint16_t active_sensors) {
  if (active_sensors > 0) {
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { CMD_THERMAL_EEPROM_REQUEST, sensor_select_high, sensor_select_low };
    interface->send(com::ComEndpoint("thermal_request"), tx_buf);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Requested transmission of EEPROM from thermal sensors but no boards have been selected");
  }
}

void ThermalSensor::cmdRequestThermalMeasurement(com::ComInterface* interface, std::uint16_t active_sensors) {
  if (active_sensors > 0) {
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { CMD_THERMAL_SCAN_REQUEST, sensor_select_high, sensor_select_low };
    interface->send(com::ComEndpoint("thermal_request"), tx_buf);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Requested thermal measurement but no boards have been selected");
  }
}

void ThermalSensor::cmdFetchThermalMeasurement(com::ComInterface* interface, std::uint16_t active_sensors) {
  if (active_sensors > 0) {
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { CMD_THERMAL_DATA_REQUEST, sensor_select_high, sensor_select_low };
    interface->send(com::ComEndpoint("thermal_request"), tx_buf);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Requested thermal measurement but no boards have been selected");
  }
}

} // namespace sensor

} // namespace eduart