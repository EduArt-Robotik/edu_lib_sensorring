#pragma once

#include <string>

#include "sensorring/device/DeviceID.hpp"
#include "sensorring/device/Measurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

template <DeviceType S> struct DeviceCapabilities;
template <> struct DeviceCapabilities<DeviceType::VL53L8> {
  using message_t = VL53L8Message;
};
template <> struct DeviceCapabilities<DeviceType::HTPA32> {
  using message_t = HTPA32Message;
};
template <> struct DeviceCapabilities<DeviceType::WS2812b> {
  using message_t = WS2812bMessage;
};

class SENSORRING_EXPORT IDevice {
public:
  virtual ~IDevice() = default;

  virtual const DeviceID& getID() const;

  void setID(const DeviceID& id);

  //virtual void sendValue(const MessageVariant& value);

  //virtual void receiveValue(const MessageVariant& value);

protected:
  DeviceID _id;
};

} // namespace device

} // namespace eduart