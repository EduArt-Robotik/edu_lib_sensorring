#pragma once

#include "BaseSensor.hpp"
#include "DeviceID.hpp"
#include "IDevice.hpp"

namespace eduart {

namespace device {

class SENSORRING_EXPORT DeviceImpl;

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

class SENSORRING_EXPORT BaseDevice : public IDevice, public BaseSensor {
public:
  BaseDevice(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable);
  virtual ~BaseDevice() = default;

  DeviceID getDeviceID() const;

  //void setEnable(bool enable);
  //bool getEnable() const;

protected:
  DeviceState _state;
  DeviceID _id;
  bool _enable;
};

} // namespace device

} // namespace eduart