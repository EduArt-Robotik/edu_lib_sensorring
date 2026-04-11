#include "device/hardware/vl53l8cx/VL53L8CX_DeviceImpl.hpp"

#include <algorithm>
#include <sensorring_transport/Protocol.hpp>

#include "interface/ComInterface.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"

using namespace eduart::transport::protocol;
using namespace eduart::transport::protocol::vl53l8cx;

namespace eduart {

namespace sensorring {

namespace device {

VL53L8CX_DeviceImpl::VL53L8CX_DeviceImpl(VL53L8CX_Device& parent, VL53L8CX_Params params, com::ComInterface* interface, unsigned int idx)
    : _parent(parent)
    , _params(params) {
  _rx_buffer_offset = 0;
  (void)interface;
  (void)idx;
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
}

VL53L8CX_DeviceImpl::~VL53L8CX_DeviceImpl() {
}

const VL53L8CX_Params& VL53L8CX_DeviceImpl::getParams() const {
  return _params;
}

std::pair<const measurement::TofMeasurement&, DeviceState> VL53L8CX_DeviceImpl::getLatestMeasurement() const {
  return { _latest_raw_measurement, _parent._error };
}

std::pair<const measurement::TofMeasurement&, DeviceState> VL53L8CX_DeviceImpl::getLatestTransformedMeasurement() const {
  return { _latest_transformed_measurement, _parent._error };
}

void VL53L8CX_DeviceImpl::onResetSensorState() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset   = 0;
  _rx_buffer_complete = false;
}

void VL53L8CX_DeviceImpl::onClearDataFlag() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset   = 0;
  _rx_buffer_complete = false;
}

void VL53L8CX_DeviceImpl::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(_parent._state_mutex);

  switch (command) {
  case MEASUREMENT_RESPONSE:
    // "Measurement done" notification
    _parent.setDataAvailableReady(true);
    break;

  case MEASUREMENT_TRANSMISSION_RESPONSE: {
    // Complete measurement data delivered by reassembly layer.
    // Expected: [frame_id, <192 bytes of point data>]
    if (data.size() >= sizeof(_rx_buffer) + 1) {
      uint8_t frame_id = data[0];
      std::copy_n(data.begin() + 1, sizeof(_rx_buffer), _rx_buffer);
      _latest_raw_measurement         = processMeasurement(frame_id, _rx_buffer, vl53l8::TOF_RESOLUTION);
      _latest_transformed_measurement = transformTofMeasurements(_latest_raw_measurement, _parent._rot_m, _parent._translation);
      _rx_buffer_complete             = false;
      _parent.setMeasurementReady(true);
    }
    break;
  }

  default:
    break;
  }
}

measurement::TofMeasurement VL53L8CX_DeviceImpl::processMeasurement(int frame_id, uint8_t* data, int len) const {
  measurement::TofMeasurement result;
  result.point_cloud.data.reserve(vl53l8::TOF_RESOLUTION);
  result.frame_id = frame_id;

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

    result.point_cloud.data.push_back(measurement::PointData({ point, point_distance, point_sigma, _params.id.index }));
  }

  result.point_cloud.data.shrink_to_fit();
  return result;
}

measurement::TofMeasurement VL53L8CX_DeviceImpl::transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation) {
  auto transformed_measurement = measurement;

  for (unsigned int i = 0; i < transformed_measurement.point_cloud.data.size(); i++) {
    transformed_measurement.point_cloud.data[i].point = (rotation * measurement.point_cloud.data[i].point) + translation;
  }

  return transformed_measurement;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
