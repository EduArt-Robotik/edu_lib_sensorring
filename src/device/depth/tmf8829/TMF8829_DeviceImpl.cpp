#include "device/depth/tmf8829/TMF8829_DeviceImpl.hpp"

#include <chrono>
#include <sensorring_transport/Protocol.hpp>
#include <thread>

#include "interface/ComInterface.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::tmf8829;

using namespace std::chrono_literals;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_DeviceImpl::TMF8829_DeviceImpl(TMF8829_Device& parent, TMF8829_Params params, com::ComInterface*, unsigned int)
    : _resolution_mode(params.resolution_mode)
    , _parent(parent)
    , _params(params) {
}

TMF8829_DeviceImpl::~TMF8829_DeviceImpl() {
}

const TMF8829_Params& TMF8829_DeviceImpl::getParams() const {
  return _params;
}

int TMF8829_DeviceImpl::getResolutionMode() {
  _resolution_mode = 0xff;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_CONFIG_GET_RESOLUTION, {});
  auto now = std::chrono::steady_clock::now();

  while (_resolution_mode == 0xff && std::chrono::steady_clock::now() - now < 100ms) { // ToDo: Add timeout parameter
    std::this_thread::sleep_for(10ms);
  }

  return _resolution_mode;
}

bool TMF8829_DeviceImpl::setResolutionMode(std::uint8_t mode) {
  bool success = true;
  success &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_CONFIG_SET_RESOLUTION, { mode });
  success &= getResolutionMode() == mode;
  return success;
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

  case PARAMETER_CONFIG_GET_RESOLUTION: {
    if (data.size() >= 1) {
      _resolution_mode = data[0];
    }
    return;
  }

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
