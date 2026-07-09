#include "device/depth/vl53l8cx/VL53L8CX_DeviceImpl.hpp"

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
      _parent._latest_measurement = processMeasurement(data);
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
  result.header.device_id   = _parent._id;
  const auto global_pose    = _parent.getGlobalPose();
  result.header.position    = global_pose.translation;
  result.header.orientation = global_pose.orientation;
  result.header.frame_id    = data[0];
  result.nr_valid_points    = data[1];
  result.resolution_x       = _parent._config.res_x;
  result.resolution_y       = _parent._config.res_y;

  result.point_cloud.data.resize(RESOLUTION);

  for (unsigned int i = 0; i < RESOLUTION; i++) {
    uint16_t distance_raw = (*((uint32_t*)(data.data() + i * 3 + 2)) >> 10) & 0x3FFF; // 14 bit
    uint16_t sigma_raw    = (*((uint32_t*)(data.data() + i * 3 + 2)) >> 0) & 0x03FF;  // 10 bit

    if (distance_raw != 0) {
      result.point_cloud.data[i].raw_distance = static_cast<double>(distance_raw) / 4.0 / 1000.0; // Factor 4 for fixed point conversion, Factor 1000 from mm to m
      result.point_cloud.data[i].sigma        = static_cast<double>(sigma_raw) / 128.0 / 1000.0;  // Factor 128 for fixed point conversion, Factor 1000 from mm to m
    }
  }

  try {
    _parent.processRawMeasurement(result.point_cloud);
    result.point_cloud.data.shrink_to_fit();
    result.header.state = DeviceState::Ok;
  } catch (const std::exception& e) {
    result.header.state = DeviceState::Error;
  }
  return result;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
