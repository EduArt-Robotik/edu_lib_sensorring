#pragma once

#include "interface/ComObserver.hpp"

#include "DeviceID.hpp"
#include "IDevice.hpp"

namespace eduart {

namespace device {

struct SENSORRING_EXPORT DeviceParams {
  DeviceID id;
  bool enabled;
};

enum class SENSORRING_EXPORT DeviceState {
  UNDEFINED,
  INITIALIZED,
  IDLE,
  ERROR,
  SHUTDOWN
};

class SENSORRING_EXPORT Device : public IDevice {
public:
  Device(const std::string& name);
  ~Device();

  DeviceID getDeviceID() const;

  bool setEnable(bool enable);
  bool getEnable() const;

  void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;
  virtual void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) = 0;

protected:
  DeviceID _id;
  bool _enable;
};

} // namespace device

} // namespace eduart