#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"

#include <sensorring_transport/Protocol.hpp>

#include "device/depth/tmf8829/TMF8829_Constants.hpp"
#include "device/depth/tmf8829/TMF8829_DeviceImpl.hpp"
#include "interface/ComManager.hpp"

using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::tmf8829;

namespace eduart {

namespace sensorring {

namespace device {

TMF8829_Device::TMF8829_Device(TMF8829_Params params, com::ComInterfaceID interface, unsigned int idx)
    : BaseDevice(DeviceID({ DeviceType::TMF8829, idx }), com::ComManager::getInstance()->getInterface(interface), com::ComEndpoint{ com::Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::TMF8829 }, params.enable)
    , DepthSensor{ tmf8829::FOV_X_DEG, tmf8829::FOV_Y_DEG, getXResolution(params.resolution_mode), getYResolution(params.resolution_mode) }
    , _impl(std::make_unique<TMF8829_DeviceImpl>(*this, params, com::ComManager::getInstance()->getInterface(interface), idx)) {
  configure();
}

TMF8829_Device::~TMF8829_Device() {
}

const TMF8829_Params& TMF8829_Device::getParams() const {
  return _impl->getParams();
}

bool TMF8829_Device::setResolutionMode(ResolutionMode mode) {
  return _impl->setResolutionMode(mode);
}

bool TMF8829_Device::getResolutionMode(ResolutionMode& mode) {
  return _impl->getResolutionMode(mode);
}

bool TMF8829_Device::setIterationsSetting(std::uint16_t k_iterations) {
  return _impl->setIterationsSetting(k_iterations);
}

bool TMF8829_Device::getIterationsSetting(std::uint16_t& k_iterations) {
  return _impl->getIterationsSetting(k_iterations);
}

bool TMF8829_Device::setResultFormat(TMF8829_ResultFormat format) {
  return _impl->setResultFormat(format);
}

bool TMF8829_Device::getResultFormat(TMF8829_ResultFormat& format) {
  return _impl->getResultFormat(format);
}

bool TMF8829_Device::setResultFullNoise(bool full_noise) {
  return _impl->setResultFullNoise(full_noise);
}

bool TMF8829_Device::getResultFullNoise(bool& full_noise) {
  return _impl->getResultFullNoise(full_noise);
}

bool TMF8829_Device::setResultXtalk(bool xtalk) {
  return _impl->setResultXtalk(xtalk);
}

bool TMF8829_Device::getResultXtalk(bool& xtalk) {
  return _impl->getResultXtalk(xtalk);
}

bool TMF8829_Device::setResultNoiseStrength(bool noise_strength) {
  return _impl->setResultNoiseStrength(noise_strength);
}

bool TMF8829_Device::getResultNoiseStrength(bool& noise_strength) {
  return _impl->getResultNoiseStrength(noise_strength);
}

bool TMF8829_Device::setResultSignalStrength(bool signal_strength) {
  return _impl->setResultSignalStrength(signal_strength);
}

bool TMF8829_Device::getResultSignalStrength(bool& signal_strength) {
  return _impl->getResultSignalStrength(signal_strength);
}

bool TMF8829_Device::setResultNrOfPeaks(std::uint8_t nr_of_peaks) {
  return _impl->setResultNrOfPeaks(nr_of_peaks);
}

bool TMF8829_Device::getResultNrOfPeaks(std::uint8_t& nr_of_peaks) {
  return _impl->getResultNrOfPeaks(nr_of_peaks);
}

bool TMF8829_Device::configure() {
  if (!getEnable()) {
    return true;
  }

  if (!setResolutionMode(getParams().resolution_mode)) {
    return false;
  }

  if (getParams().k_iterations != 0 && !setIterationsSetting(getParams().k_iterations)) {
    return false;
  }

  if (!setResultFormat(getParams().result_format)) {
    return false;
  }

  return true;
}

void TMF8829_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) {
  if (!_impl) { // ToDo: add as general check in BaseDevice to avoid this in all devices
    return;
  }
  _impl->comCallback(source, command, data);
}

std::future<bool> TMF8829_Device::requestMeasurementAsync(const std::vector<TMF8829_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    (void)timeout;
    struct InterfaceGroup {
      std::vector<TMF8829_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;
    std::vector<std::future<bool> > futures;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);
        futures.emplace_back(dev->beginDataAvailableWait());
        group.active_sensors |= (1u << dev->getIdx());
      }
    }

    if (groups.empty()) {
      return false;
    }

    static std::atomic<std::uint8_t> request_count{ 0 };
    std::uint8_t count = request_count.fetch_add(1, std::memory_order_relaxed);

    for (auto& [iface, group] : groups) {
      if (group.devices.empty()) {
        continue;
      }
      unsigned int active_sensors = group.active_sensors;
      uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
      uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
      std::vector<uint8_t> tx_buf{ count, sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::TMF8829 }, MEASUREMENT_REQUEST, tx_buf);
    }

    for (auto& fut : futures) {
      try {
        if (!fut.get()) {
          return false;
        }
      } catch (const std::future_error&) {
        return false;
      }
    }

    return true;
  });
}

std::future<bool> TMF8829_Device::fetchMeasurementAsync(std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [this, timeout]() {
    if (!getEnable()) {
      return false;
    }

    clearDataFlag();
    auto fut = beginMeasurementWait();

    _interface->send(com::ComEndpoint{ com::Direction::Input, static_cast<std::uint8_t>(_hw_idx + 1), devbyte::TMF8829 }, MEASUREMENT_TRANSMISSION_REQUEST, {});

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    if (fut.wait_until(deadline) != std::future_status::ready) {
      return false;
    }
    return fut.get();
  });
}

} // namespace device

} // namespace sensorring

} // namespace eduart
