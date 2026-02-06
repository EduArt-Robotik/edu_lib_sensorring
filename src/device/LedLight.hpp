#pragma once

#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/IDevice.hpp"
#include "sensorring/types/LightMode.hpp"

namespace eduart {

namespace device {

struct SetLight {
  struct Request {
    com::ComInterface* interface;
    light::LightMode mode;
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
  };
  struct Response {
    bool ok;
  };
};

struct SyncLight {
  struct Request {
    com::ComInterface* interface;
  };
  struct Response {
    bool ok;
  };
};

class LedLight : public IDevice {
public:
  LedLight(LightParams params, com::ComInterface* interface);
  ~LedLight();

  const LightParams& getParams() const;

  static SetLight::Response setLight(const SetLight::Request& request);
  static SyncLight::Response syncLight(const SyncLight::Request& request);

private:
  const LightParams _params;

  int _canid_in;
  int _canid_out;
};

} // namespace device

} // namespace eduart