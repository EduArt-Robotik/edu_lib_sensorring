#include "device/depth/tmf8829/TMF8829_DeviceImpl.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "interface/ComInterface.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::tmf8829;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_DeviceImpl::TMF8829_DeviceImpl(TMF8829_Device& parent, TMF8829_Params params, com::ComInterface*, unsigned int)
    : _parent(parent)
    , _params(params) {
}

TMF8829_DeviceImpl::~TMF8829_DeviceImpl() {
}

const TMF8829_Params& TMF8829_DeviceImpl::getParams() const {
  return _params;
}

std::pair<const measurement::DepthMeasurement&, DeviceState> TMF8829_DeviceImpl::getLatestMeasurement() const {
  return { _latest_raw_measurement, _parent._state };
}

std::pair<const measurement::DepthMeasurement&, DeviceState> TMF8829_DeviceImpl::getLatestTransformedMeasurement() const {
  return { _latest_transformed_measurement, _parent._state };
}

void TMF8829_DeviceImpl::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(_parent._state_mutex);

  switch (command) {
  case MEASUREMENT_RESPONSE:
    // "Measurement done" notification
    _parent.setDataAvailableReady(true);
    break;

  case MEASUREMENT_TRANSMISSION_RESPONSE: {
    // Complete measurement data delivered by reassembly layer.
    // Expected: [frame_id, nr of valid points, <192 bytes of point data>]
    if (data.size() >= (tmf8829::TOF_RESOLUTION * 3 + 2)) {
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

measurement::DepthMeasurement TMF8829_DeviceImpl::processMeasurement(const std::vector<uint8_t>& data) const {
  measurement::DepthMeasurement result;
  result.point_cloud.data.reserve(tmf8829::TOF_RESOLUTION);
  result.frame_id        = data[0];
  result.nr_valid_points = data[1];

  uint16_t distance_raw = 0;
  uint16_t sigma_raw    = 0;

  for (int i = 0; i < tmf8829::TOF_RESOLUTION; i++) {
    distance_raw = (*((uint32_t*)(data.data() + i * 3 + 2)) >> 10) & 0x3FFF; // 14 bit
    sigma_raw    = (*((uint32_t*)(data.data() + i * 3 + 2)) >> 0) & 0x03FF;  // 10 bit

    math::Vector3 point   = { 0, 0, 0 };
    double point_distance = -1;
    double point_sigma    = -1;

    if (distance_raw != 0) {
      point_distance = (double)distance_raw / 4.0F / 1000.0F; // Factor 4 for fixed point conversion, Factor 1000 from mm to m
      point_sigma    = (double)sigma_raw / 128.0 / 1000.0F;   // Factor 128 for fixed point conversion, Factor 1000 from mm to m

      point.x() = point_distance * tmf8829::lut_tan_x[i];
      point.y() = point_distance * tmf8829::lut_tan_y[i];
      point.z() = point_distance;
    }

    result.point_cloud.data.push_back(measurement::PointData({ point, point_distance, point_sigma, _params.id.index }));
  }

  result.point_cloud.data.shrink_to_fit();
  return result;
}

measurement::DepthMeasurement TMF8829_DeviceImpl::transformMeasurement(const measurement::DepthMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation) {
  auto transformed_measurement = measurement;
  transformed_measurement.point_cloud = measurement::PointCloud::transform(measurement.point_cloud, rotation, translation);
  return transformed_measurement;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
