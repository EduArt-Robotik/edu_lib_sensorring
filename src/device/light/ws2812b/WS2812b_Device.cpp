#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"

#include <algorithm>
#include <cmath>
#include <sensorring_transport/Protocol.hpp>
#include <thread>

#include "interface/ComManager.hpp"
#include "sensorring/logger/Logger.hpp"
#include "sensorring_transport/ByteOperations.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::ws2812b;

namespace eduart {

namespace sensorring {

namespace device {

namespace {

bool distanceMatches(double actual, double requested) {
  // Allow float32 rounding
  return std::abs(actual - requested) <= std::max(1.0e-6, std::abs(requested) * 1.0e-6);
}

} // namespace

WS2812b_Device::WS2812b_Device(WS2812b_Params params, com::ComInterfaceID interface, unsigned int idx)
    : WS2812b_Device(std::move(params), com::ComManager::getInstance()->getInterface(interface), idx) {
}

WS2812b_Device::WS2812b_Device(WS2812b_Params params, com::ComInterface* interface, unsigned int idx)
    : Light(DeviceID({ DeviceType::WS2812b, idx }), interface, com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::WS2812B })
    , _params(std::move(params))
    , _last_setting({}) {
  _params.validateDistanceMapRange();
  _params.validateDistanceMapBrightness();
}

bool WS2812b_Device::configure() {
  if (!setDistanceMapRange(_params.distance_map_lower_m, _params.distance_map_upper_m)) {
    return false;
  }
  if (!setDistanceMapMaxBrightness(_params.distance_map_max_brightness)) {
    return false;
  }
  setLight(_last_setting.mode, _last_setting.red, _last_setting.green, _last_setting.blue);
  return true;
}

bool WS2812b_Device::setDistanceMapRange(double lower_m, double upper_m) {
  std::lock_guard<std::mutex> lock(_parameter_mutex);

  WS2812b_Params proposed       = _params;
  proposed.distance_map_lower_m = lower_m;
  proposed.distance_map_upper_m = upper_m;
  try {
    proposed.validateDistanceMapRange();
  } catch (const std::invalid_argument& e) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, e.what());
    return false;
  }

  std::vector<uint8_t> tx_buf(8);
  transport::ByteOperations::writeFloat(tx_buf, 0, static_cast<float>(lower_m));
  transport::ByteOperations::writeFloat(tx_buf, 4, static_cast<float>(upper_m));
  bool success = _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::WS2812B }, PARAMETER_SET_DISTANCE_MAP_RANGE, tx_buf);

  std::vector<uint8_t> data;
  if (success) {
    success = requestParameter(PARAMETER_GET_DISTANCE_MAP_RANGE, data);
  }
  if (success) {
    success = distanceMatches(transport::ByteOperations::readFloat(data, 0), lower_m) && distanceMatches(transport::ByteOperations::readFloat(data, 4), upper_m);
  }

  if (success) {
    _params.distance_map_lower_m = lower_m;
    _params.distance_map_upper_m = upper_m;
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set WS2812b distance map range on board " + std::to_string(getDeviceID().index) + " to [" + std::to_string(lower_m) + ", " + std::to_string(upper_m) + "]");
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set WS2812b distance map range on board " + std::to_string(getDeviceID().index) + " to [" + std::to_string(lower_m) + ", " + std::to_string(upper_m) + "]");
  }

  return success;
}

bool WS2812b_Device::getDistanceMapRange(double& lower_m, double& upper_m) {
  std::lock_guard<std::mutex> lock(_parameter_mutex);
  std::vector<uint8_t> data;
  if (!requestParameter(PARAMETER_GET_DISTANCE_MAP_RANGE, data)) {
    return false;
  }

  lower_m = transport::ByteOperations::readFloat(data, 0);
  upper_m = transport::ByteOperations::readFloat(data, 4);
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got WS2812b distance map range on board " + std::to_string(getDeviceID().index));
  return true;
}

bool WS2812b_Device::setDistanceMapMaxBrightness(std::int64_t brightness) {
  std::lock_guard<std::mutex> lock(_parameter_mutex);

  WS2812b_Params proposed              = _params;
  proposed.distance_map_max_brightness = brightness;
  try {
    proposed.validateDistanceMapBrightness();
  } catch (const std::invalid_argument& e) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, e.what());
    return false;
  }

  bool success = _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::WS2812B }, PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS, { static_cast<std::uint8_t>(brightness) });

  std::vector<uint8_t> data;
  if (success) {
    success = requestParameter(PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS, data);
  }
  if (success) {
    success = (data[0] == brightness);
  }

  if (success) {
    _params.distance_map_max_brightness = brightness;
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set WS2812b distance map brightness on board " + std::to_string(getDeviceID().index) + " to " + std::to_string(brightness));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set WS2812b distance map brightness on board " + std::to_string(getDeviceID().index) + " to " + std::to_string(brightness));
  }

  return success;
}

bool WS2812b_Device::getDistanceMapMaxBrightness(std::int64_t& brightness) {
  std::lock_guard<std::mutex> lock(_parameter_mutex);
  std::vector<uint8_t> data;
  if (!requestParameter(PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS, data)) {
    return false;
  }

  brightness = data[0];
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got WS2812b distance map brightness on board " + std::to_string(getDeviceID().index) + ": " + std::to_string(brightness));
  return true;
}

bool WS2812b_Device::requestParameter(std::uint8_t command, std::vector<uint8_t>& data) {
  {
    std::lock_guard<std::mutex> lock(_state_mutex);
    _pending_parameter = command;
    _got_update        = false;
  }

  bool success = _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::WS2812B }, command, {});
  if (success) {
    auto now = std::chrono::steady_clock::now();
    while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
      std::this_thread::sleep_for(GET_PARAMETER_SLEEP);
    }
  }

  {
    std::lock_guard<std::mutex> lock(_state_mutex);
    _pending_parameter = 0;
    success            = success && _got_update;
    if (success) {
      data = _parameter_data;
    }
  }

  if (!success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get WS2812b parameter " + std::to_string(command) + " on board " + std::to_string(getDeviceID().index));
  }
  return success;
}

void WS2812b_Device::setLight(LightMode mode, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  _last_setting = { mode, r, g, b };

  auto* iface     = _interface;
  auto board_addr = static_cast<std::uint8_t>(_hw_idx + 1);
  execute("setLight", [mode, r, g, b, iface, board_addr]() {
    std::uint8_t mode_cmd       = static_cast<uint8_t>(mode);
    std::vector<uint8_t> tx_buf = { mode_cmd, r, g, b };
    iface->send(com::ComEndpoint{ com::Direction::Input, board_addr, devbyte::WS2812B }, SET_LED_MODE, tx_buf);
  });
}

void WS2812b_Device::syncLight() {
  globalExecute([]() {
    std::vector<uint8_t> tx_buf = {};
    for (auto& interface : com::ComManager::getInstance()->getInterfaces()) {
      interface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::WS2812B }, SYNCHRONIZE, tx_buf);
    }
  });
}

void WS2812b_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(_state_mutex);
  if (command != _pending_parameter || _got_update) {
    return;
  }

  switch (command) {
  case PARAMETER_GET_DISTANCE_MAP_RANGE: {
    if (data.size() != 8) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid WS2812b distance map range response on board " + std::to_string(getDeviceID().index) + ": expected 8 bytes.");
      return;
    }
    const float lower_m = transport::ByteOperations::readFloat(data, 0);
    const float upper_m = transport::ByteOperations::readFloat(data, 4);
    if (!std::isfinite(lower_m) || !std::isfinite(upper_m) || lower_m < 0.0F || upper_m <= lower_m) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid WS2812b distance map range values on board " + std::to_string(getDeviceID().index));
      return;
    }
    break;
  }

  case PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS:
    if (data.size() != 1) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid WS2812b distance map brightness response on board " + std::to_string(getDeviceID().index) + ": expected 1 byte.");
      return;
    }
    break;

  default:
    return;
  }

  _parameter_data = data;
  _got_update     = true;
}

} // namespace device

} // namespace sensorring

} // namespace eduart
