#include "device/depth/tmf8829/TMF8829_DeviceImpl.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "interface/ComInterface.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/logger/Logger.hpp"

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
  return { _latest_measurement, _parent._state };
}

void TMF8829_DeviceImpl::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(_parent._state_mutex);

  switch (command) {
  case MEASUREMENT_RESPONSE:
    // "Measurement done" notification
    _parent.setDataAvailableReady(true);
    return;

  case MEASUREMENT_TRANSMISSION_RESPONSE: {
    try {
      _latest_measurement                  = TMF8829_Measurement::fromBuffer(data);
      _latest_measurement.header.device_id = _parent.getDeviceID();

      if (_latest_measurement.header.state == device::DeviceState::Ok) {
        _parent.processRawMeasurement(_parent._lut_x, _parent._lut_y, _latest_measurement.point_cloud);
      }
      _parent.setMeasurementReady(true);
      return;
    } catch (std::exception& e) {
      _parent.setMeasurementReady(false);
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception while processing TMF8829 measurement data: " + std::string(e.what()));
    }
    break;
  }

  default:
    break;
  }
}

} // namespace device

} // namespace sensorring

} // namespace eduart
