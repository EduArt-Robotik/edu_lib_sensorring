#include "device/depth/tmf8829/TMF8829_DeviceImpl.hpp"

#include <chrono>
#include <sensorring_transport/Protocol.hpp>
#include <string>
#include <thread>

#include "interface/ComInterface.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/logger/Logger.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::tmf8829;

using namespace std::chrono_literals;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_DeviceImpl::TMF8829_DeviceImpl(TMF8829_Device& parent, TMF8829_Params params, com::ComInterface*, unsigned int)
    : _got_update(false)
    , _params(params)
    , _parent(parent) {
}

TMF8829_DeviceImpl::~TMF8829_DeviceImpl() {
}

const TMF8829_Params& TMF8829_DeviceImpl::getParams() const {
  return _params;
}

bool TMF8829_DeviceImpl::isParamCombinationValid(const TMF8829_Params& params) const {
  if (!params.isResultSizeValid()) {
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Error,
        "Requested parameter combination of TMF8829 on board " + std::to_string(_parent.getDeviceID().index) + " exceeds the maximum result frame size (" + std::to_string(params.calculateResultFrameSize()) + " > 8192 bytes).");
    return false;
  }
  return true;
}

bool TMF8829_DeviceImpl::setResolutionMode(ResolutionMode mode) {
  TMF8829_Params proposed  = _params;
  proposed.resolution_mode = mode;
  bool success             = isParamCombinationValid(proposed);

  if (success) {
    success &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_CONFIG_SET_RESOLUTION, { static_cast<std::uint8_t>(mode) });

    ResolutionMode current_mode;
    success &= getResolutionMode(current_mode);
    success &= (current_mode == mode);
  }

  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set TMF8829 resolution on board " + std::to_string(_parent.getDeviceID().index) + " to mode " + toString(mode));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set TMF8829 resolution on board " + std::to_string(_parent.getDeviceID().index) + " to mode " + toString(mode));
  }

  return success;
}

bool TMF8829_DeviceImpl::getResolutionMode(ResolutionMode& mode) {
  _got_update = false;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_CONFIG_GET_RESOLUTION, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(10ms);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got TMF8829 resolution update on board " + std::to_string(_parent.getDeviceID().index) + ": mode " + toString(_params.resolution_mode));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get TMF8829 resolution on board " + std::to_string(_parent.getDeviceID().index));
    return false;
  }

  mode = _params.resolution_mode;
  return true;
}

bool TMF8829_DeviceImpl::getResultFormat(TMF8829_ResultFormat& format) {
  bool success = true;

  do {
    success = getResultFullNoise(format.full_noise);
    if (!success)
      break;

    success = getResultXtalk(format.xtalk);
    if (!success)
      break;

    success = getResultNoiseStrength(format.noise_strength);
    if (!success)
      break;

    success = getResultSignalStrength(format.signal_strength);
    if (!success)
      break;

    success = getResultNrOfPeaks(format.nr_of_peaks);
    if (!success)
      break;

  } while (false);

  format = _params.result_format;
  return success;
}

bool TMF8829_DeviceImpl::setResultFormat(TMF8829_ResultFormat format) {
  TMF8829_Params proposed = _params;
  proposed.result_format  = format;
  bool success            = isParamCombinationValid(proposed);

  if (success)
    do {
      success = setResultFullNoise(format.full_noise);
      if (!success)
        break;

      success = setResultXtalk(format.xtalk);
      if (!success)
        break;

      success = setResultNoiseStrength(format.noise_strength);
      if (!success)
        break;

      success = setResultSignalStrength(format.signal_strength);
      if (!success)
        break;

      success = setResultNrOfPeaks(format.nr_of_peaks);
      if (!success)
        break;

    } while (false);

  return success;
}

bool TMF8829_DeviceImpl::setResultFullNoise(bool full_noise) {
  TMF8829_Params proposed           = _params;
  proposed.result_format.full_noise = full_noise;
  bool success                      = isParamCombinationValid(proposed);

  if (success) {
    success &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_SET_FULL_NOISE, { static_cast<std::uint8_t>(full_noise) });

    bool current_full_noise;
    success &= getResultFullNoise(current_full_noise);
    success &= (current_full_noise == full_noise);
  }

  std::string enable_str = full_noise ? "true" : "false";
  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set TMF8829 full noise on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set TMF8829 full noise on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  }

  return success;
}

bool TMF8829_DeviceImpl::getResultFullNoise(bool& full_noise) {
  _got_update = false;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_GET_FULL_NOISE, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(10ms);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got TMF8829 full noise update on board " + std::to_string(_parent.getDeviceID().index) + ": full noise " + (_params.result_format.full_noise ? "true" : "false"));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get TMF8829 full noise on board " + std::to_string(_parent.getDeviceID().index));
    return false;
  }

  full_noise = _params.result_format.full_noise;
  return true;
}

bool TMF8829_DeviceImpl::setResultXtalk(bool xtalk) {
  TMF8829_Params proposed      = _params;
  proposed.result_format.xtalk = xtalk;
  bool success                 = isParamCombinationValid(proposed);

  if (success) {
    success &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_SET_XTALK, { static_cast<std::uint8_t>(xtalk) });

    bool current_xtalk;
    success &= getResultXtalk(current_xtalk);
    success &= (current_xtalk == xtalk);
  }

  std::string enable_str = xtalk ? "true" : "false";
  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set TMF8829 xtalk on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set TMF8829 xtalk on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  }

  return success;
}

bool TMF8829_DeviceImpl::getResultXtalk(bool& xtalk) {
  _got_update = false;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_GET_XTALK, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(10ms);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got TMF8829 xtalk update on board " + std::to_string(_parent.getDeviceID().index) + ": xtalk " + (_params.result_format.xtalk ? "true" : "false"));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get TMF8829 xtalk on board " + std::to_string(_parent.getDeviceID().index));
    return false;
  }

  xtalk = _params.result_format.xtalk;
  return true;
}

bool TMF8829_DeviceImpl::setResultNoiseStrength(bool noise_strength) {
  TMF8829_Params proposed               = _params;
  proposed.result_format.noise_strength = noise_strength;
  bool success                          = isParamCombinationValid(proposed);

  if (success) {
    success &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_SET_NOISE_STRENGTH, { static_cast<std::uint8_t>(noise_strength) });

    bool current_noise_strength;
    success &= getResultNoiseStrength(current_noise_strength);
    success &= (current_noise_strength == noise_strength);
  }

  std::string enable_str = noise_strength ? "true" : "false";
  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set TMF8829 noise strength on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set TMF8829 noise strength on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  }

  return success;
}

bool TMF8829_DeviceImpl::getResultNoiseStrength(bool& noise_strength) {
  _got_update = false;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_GET_NOISE_STRENGTH, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(10ms);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Debug, "Got TMF8829 noise strength update on board " + std::to_string(_parent.getDeviceID().index) + ": noise strength " + (_params.result_format.noise_strength ? "true" : "false"));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get TMF8829 noise strength on board " + std::to_string(_parent.getDeviceID().index));
    return false;
  }

  noise_strength = _params.result_format.noise_strength;
  return true;
}

bool TMF8829_DeviceImpl::setResultSignalStrength(bool signal_strength) {
  TMF8829_Params proposed                = _params;
  proposed.result_format.signal_strength = signal_strength;
  bool success                           = isParamCombinationValid(proposed);

  if (success) {
    success
        &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_SET_SIGNAL_STRENGTH, { static_cast<std::uint8_t>(signal_strength) });

    bool current_signal_strength;
    success &= getResultSignalStrength(current_signal_strength);
    success &= (current_signal_strength == signal_strength);
  }

  std::string enable_str = signal_strength ? "true" : "false";
  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set TMF8829 signal strength on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set TMF8829 signal strength on board " + std::to_string(_parent.getDeviceID().index) + " to " + enable_str);
  }

  return success;
}

bool TMF8829_DeviceImpl::getResultSignalStrength(bool& signal_strength) {
  _got_update = false;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_GET_SIGNAL_STRENGTH, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(10ms);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Debug, "Got TMF8829 signal strength update on board " + std::to_string(_parent.getDeviceID().index) + ": signal strength " + (_params.result_format.signal_strength ? "true" : "false"));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get TMF8829 signal strength on board " + std::to_string(_parent.getDeviceID().index));
    return false;
  }

  signal_strength = _params.result_format.signal_strength;
  return true;
}

bool TMF8829_DeviceImpl::setResultNrOfPeaks(std::uint8_t nr_of_peaks) {
  TMF8829_Params proposed            = _params;
  proposed.result_format.nr_of_peaks = nr_of_peaks;
  bool success                       = isParamCombinationValid(proposed);

  if (success) {
    success &= _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_SET_NR_PEAKS, { static_cast<std::uint8_t>(nr_of_peaks) });

    std::uint8_t current_nr_of_peaks;
    success &= getResultNrOfPeaks(current_nr_of_peaks);
    success &= (current_nr_of_peaks == nr_of_peaks);
  }

  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set TMF8829 number of peaks on board " + std::to_string(_parent.getDeviceID().index) + " to " + std::to_string(nr_of_peaks));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set TMF8829 number of peaks on board " + std::to_string(_parent.getDeviceID().index) + " to " + std::to_string(nr_of_peaks));
  }

  return success;
}

bool TMF8829_DeviceImpl::getResultNrOfPeaks(std::uint8_t& nr_of_peaks) {
  _got_update = false;

  _parent._interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_parent._idx + 1), devbyte::TMF8829 }, tmf8829::PARAMETER_RESULT_GET_NR_PEAKS, {});
  auto now = std::chrono::steady_clock::now();

  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(10ms);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got TMF8829 number of peaks update on board " + std::to_string(_parent.getDeviceID().index) + ": number of peaks " + std::to_string(_params.result_format.nr_of_peaks));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get TMF8829 number of peaks on board " + std::to_string(_parent.getDeviceID().index));
    return false;
  }

  nr_of_peaks = _params.result_format.nr_of_peaks;
  return true;
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
      _params.resolution_mode = static_cast<ResolutionMode>(data[0]);
      _got_update             = true;
    }
    return;
  }

  case PARAMETER_RESULT_GET_FULL_NOISE: {
    if (data.size() >= 1) {
      _params.result_format.full_noise = data[0] != 0;
      _got_update                      = true;
    }
    return;
  }

  case PARAMETER_RESULT_GET_XTALK: {
    if (data.size() >= 1) {
      _params.result_format.xtalk = data[0] != 0;
      _got_update                 = true;
    }
    return;
  }

  case PARAMETER_RESULT_GET_NOISE_STRENGTH: {
    if (data.size() >= 1) {
      _params.result_format.noise_strength = data[0] != 0;
      _got_update                          = true;
    }
    return;
  }

  case PARAMETER_RESULT_GET_SIGNAL_STRENGTH: {
    if (data.size() >= 1) {
      _params.result_format.signal_strength = data[0] != 0;
      _got_update                           = true;
    }
    return;
  }

  case PARAMETER_RESULT_GET_NR_PEAKS: {
    if (data.size() >= 1) {
      _params.result_format.nr_of_peaks = data[0];
      _got_update                       = true;
    }
    return;
  }

  case MEASUREMENT_TRANSMISSION_RESPONSE: {
    try {
      _parent._latest_measurement                  = TMF8829_Measurement::fromBuffer(data);
      _parent._latest_measurement.header.device_id = _parent.getDeviceID();

      if (_parent._latest_measurement.header.state == device::DeviceState::Ok) {
        _parent.processRawMeasurement(_parent._lut_x, _parent._lut_y, _parent._latest_measurement.point_cloud);
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
