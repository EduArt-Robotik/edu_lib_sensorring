#pragma once

#include "interface/ComInterface.hpp"
#include "BaseSensor.hpp"

#include "sensorring/Parameters.hpp"

namespace eduart {

namespace sensor {

class LedLight {
public:
  LedLight(LightParams params);
  ~LedLight();

  const LightParams& getParams() const;

  static void cmdSyncLight(com::ComInterface* interface);
  static void cmdSetLight(com::ComInterface* interface, light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);

private:
  const LightParams _params;

  int _canid_in;
  int _canid_out;
};

} // namespace sensor

} // namespace eduart