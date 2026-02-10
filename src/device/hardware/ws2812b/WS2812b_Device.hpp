#pragma once

#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/types/LightMode.hpp"

namespace eduart {

namespace device {

struct WS2812b_Device : BaseDevice {
public:
  WS2812b_Device(WS2812b_Params params, com::ComInterface* interface);
  ~WS2812b_Device();

  const WS2812b_Params& getParams() const;

  // Simple static helpers to control all WS2812b devices on the bus.
  static bool setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);
  static bool syncLight();

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  const WS2812b_Params _params;
};

} // namespace device

} // namespace eduart
