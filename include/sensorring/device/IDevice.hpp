#pragma once

#include <string>

#include "sensorring/device/DeviceID.hpp"
#include "sensorring/device/Measurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

template <DeviceType S> struct DeviceCapability;
template <> struct DeviceCapability<DeviceType::VL53L8CX> {
  using message_t = VL53L8Message;
};
template <> struct DeviceCapability<DeviceType::HTPA32> {
  using message_t = HTPA32Message;
};
template <> struct DeviceCapability<DeviceType::WS2812b> {
  using message_t = WS2812bMessage;
};

class SENSORRING_EXPORT IDevice {
public:
  virtual ~IDevice() = default;

  virtual const DeviceID& getID() const noexcept;

  void setID(const DeviceID& id) noexcept;

  const bool hasCapability(DeviceCapability capability) const noexcept;

  const std::vector<DeviceCapability>& getCapabilities() const noexcept;

  //virtual void sendValue(const MessageVariant& value);

  //virtual void receiveValue(const MessageVariant& value);

protected:
  DeviceID _id;

  std::vector<DeviceCapability> _capabilities_vec;
};

} // namespace device

} // namespace eduart