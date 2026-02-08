#pragma once

#include "interface/ComObserver.hpp"
#include "sensorring/device/DeviceID.hpp"

namespace eduart {

namespace device {

class DeviceImpl : public com::ComObserver {
public:
  DeviceImpl(DeviceID id);

  ~DeviceImpl();

  DeviceID getDeviceID() const;

  void setEnable(bool enable);

  bool getEnable() const;

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

protected:
  DeviceID _id;

  bool _enable;
};

} // namespace device

} // namespace eduart