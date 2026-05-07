#include "device/depth/vl53l8cx/VL53L8CX_DeviceImpl.hpp"

#include <algorithm>
#include <sensorring_transport/Protocol.hpp>

#include "interface/ComInterface.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Device.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::vl53l8cx;

namespace eduart {

namespace sensorring {

namespace device {

VL53L8CX_DeviceImpl::VL53L8CX_DeviceImpl(VL53L8CX_Device& parent, VL53L8CX_Params params, com::ComInterface*, unsigned int)
    : _parent(parent)
    , _params(params) {
}

VL53L8CX_DeviceImpl::~VL53L8CX_DeviceImpl() {
}

const VL53L8CX_Params& VL53L8CX_DeviceImpl::getParams() const {
  return _params;
}

std::pair<const measurement::DepthMeasurement&, DeviceState> VL53L8CX_DeviceImpl::getLatestMeasurement() const {
  return { _latest_raw_measurement, _parent._state };
}

std::pair<const measurement::DepthMeasurement&, DeviceState> VL53L8CX_DeviceImpl::getLatestTransformedMeasurement() const {
  return { _latest_transformed_measurement, _parent._state };
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
    // Expected: [frame_id, nr of valid points, <192 bytes of point data>]
    if (data.size() >= (RESOLUTION * 3 + 2)) {
      _latest_raw_measurement         = processMeasurement(data);
      _latest_transformed_measurement = transformMeasurement(_latest_raw_measurement, _parent._rot_m, _parent._translation);
      _parent.setMeasurementReady(true);
    }
    break;
  }

  default:
    break;
  }
}

measurement::DepthMeasurement VL53L8CX_DeviceImpl::processMeasurement(const std::vector<uint8_t>& data) const {
  measurement::DepthMeasurement result;
  result.frame_id        = data[0];
  result.nr_valid_points = data[1];

  std::vector<RawPointInput> raw_points;
  raw_points.reserve(RESOLUTION);

  for (unsigned int i = 0; i < RESOLUTION; i++) {
    uint16_t distance_raw = (*((uint32_t*)(data.data() + i * 3 + 2)) >> 10) & 0x3FFF; // 14 bit
    uint16_t sigma_raw    = (*((uint32_t*)(data.data() + i * 3 + 2)) >> 0) & 0x03FF;  // 10 bit

    RawPointInput raw;
    if (distance_raw != 0) {
      raw.distance = static_cast<double>(distance_raw) / 4.0 / 1000.0; // Factor 4 for fixed point conversion, Factor 1000 from mm to m
      raw.sigma    = static_cast<double>(sigma_raw) / 128.0 / 1000.0;  // Factor 128 for fixed point conversion, Factor 1000 from mm to m
    }
    raw_points.push_back(raw);
  }

  _parent.transformMeasurementToPointCloud(_parent._lut_x, _parent._lut_y, raw_points, result.point_cloud, _params.id.index);
  result.point_cloud.data.shrink_to_fit();
  return result;
}

measurement::DepthMeasurement VL53L8CX_DeviceImpl::transformMeasurement(const measurement::DepthMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation) {
  auto transformed_measurement = measurement;
  transformed_measurement.point_cloud = measurement::PointCloud::transform(measurement.point_cloud, rotation, translation);
  return transformed_measurement;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
