#include "TofSensor.hpp"

#include <algorithm>
#include <cstring>

#include "interface/can/canprotocol.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensor {

TofSensor::TofSensor(TofSensorParams params, com::ComInterface* interface, std::size_t idx)
    : BaseSensor(interface, com::ComEndpoint("tof" + std::to_string(idx) + "_data"), idx, params.enable)
    , _params(params) {

  _rx_buffer_offset = 0;
  _interface->addToFSensorToEndpointMap(idx);
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
}

TofSensor::~TofSensor() {
}

const TofSensorParams& TofSensor::getParams() const {
  return _params;
}

std::pair<const measurement::TofMeasurement&, SensorState> TofSensor::getLatestRawMeasurement() const {
  return { _latest_raw_measurement, _error };
}

std::pair<const measurement::TofMeasurement&, SensorState> TofSensor::getLatestTransformedMeasurement() const {
  return { _latest_transformed_measurement, _error };
}

void TofSensor::onResetSensorState() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset = 0;
}

void TofSensor::onClearDataFlag() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset = 0;
}

void TofSensor::canCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  std::size_t msg_size = data.size();

  // point data msg
  if (msg_size == 48) {
    if ((_rx_buffer_offset + msg_size) <= (int)sizeof(_rx_buffer)) {
      std::copy_n(data.begin(), msg_size, (uint8_t*)&_rx_buffer + _rx_buffer_offset);
      _rx_buffer_offset += msg_size;
      //_new_data_in_buffer_flag = true;
      _new_data_available_flag = false;

      // got all 4 raw data messages
      if (_rx_buffer_offset >= sizeof(_rx_buffer)) {
        _new_data_in_buffer_flag = true;
      }
    } else {
      _error = SensorState::ReceiveError;
    }

    // transmission complete message
  } else if (msg_size == 2) {
    if (_new_data_in_buffer_flag) {
      _latest_raw_measurement         = processMeasurement(data[1], _rx_buffer, vl53l8::TOF_RESOLUTION);
      _latest_transformed_measurement = transformTofMeasurements(_latest_raw_measurement, _rot_m, _translation);
      _new_data_in_buffer_flag        = false;
      _new_measurement_ready_flag     = true;
    }

    // data available message
  } else if (msg_size == 1) {
    _new_data_available_flag = true;
  }
}

measurement::TofMeasurement TofSensor::processMeasurement(int frame_id, uint8_t* data, int len) const {
  measurement::TofMeasurement measurement;
  measurement.point_cloud.data.reserve(vl53l8::TOF_RESOLUTION);
  measurement.frame_id = frame_id;

  uint16_t distance_raw = 0;
  uint16_t sigma_raw    = 0;

  for (int i = 0; i < len; i++) {
    distance_raw = (*((uint32_t*)(data + i * 3)) >> 10) & 0x3FFF; // 14 bit
    sigma_raw    = (*((uint32_t*)(data + i * 3)) >> 0) & 0x03FF;  // 10 bit

    math::Vector3 point   = { 0, 0, 0 };
    double point_distance = -1;
    double point_sigma    = -1;

    if (distance_raw != 0) {
      point_distance = (double)distance_raw / 4.0F / 1000.0F; // Factor 4 for fixed point conversion, Factor 1000 from mm to m
      point_sigma    = (double)sigma_raw / 128.0 / 1000.0F;   // Factor 128 for fixed point conversion, Factor 1000 from mm to m

      point.x() = point_distance * vl53l8::lut_tan_x[i];
      point.y() = point_distance * vl53l8::lut_tan_y[i];
      point.z() = point_distance;
    }

    measurement.point_cloud.data.push_back(measurement::PointData({ point, point_distance, point_sigma, _params.user_idx }));
  }

  measurement.point_cloud.data.shrink_to_fit();
  return measurement;
}

void TofSensor::cmdRequestTofMeasurement(com::ComInterface* interface, std::uint16_t active_sensors) {
  if (active_sensors > 0) {
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { CMD_TOF_SCAN_REQUEST, sensor_select_high, sensor_select_low };
    interface->send(com::ComEndpoint("tof_request"), tx_buf);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Requested ToF measurement but no boards have been selected");
  }
}

void TofSensor::cmdFetchTofMeasurement(com::ComInterface* interface, std::uint16_t active_sensors) {
  if (active_sensors > 0) {
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { sensor_select_high, sensor_select_low };
    interface->send(com::ComEndpoint("tof_request"), tx_buf);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Requested ToF measurement but no boards have been selected");
  }
}

measurement::TofMeasurement TofSensor::transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation) {

  auto transformed_measurement = measurement;

  for (unsigned int i = 0; i < transformed_measurement.point_cloud.data.size(); i++) {
    transformed_measurement.point_cloud.data[i].point = (rotation * measurement.point_cloud.data[i].point) + translation;
  }

  return transformed_measurement;
}

} // namespace sensor

} // namespace eduart