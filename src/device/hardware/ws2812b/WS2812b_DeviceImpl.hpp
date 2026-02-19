#pragma once

#include <cstdint>
#include <vector>

#include "sensorring/device/hardware/ws2812b/WS2812b_Params.hpp"
#include "sensorring/types/LightMode.hpp"
#include "sensorring/interface/ComEndpoint.hpp"

namespace eduart {

namespace com {
class ComInterface;
} // namespace com

namespace device {

class WS2812b_Device;

/** 
 * @class WS2812b_DeviceImpl
 * @brief Implementation class for WS2812b_Device hiding private members.
 *
 * This class contains the actual state and behaviour for the WS2812b device.
 * The public `WS2812b_Device` front-end forwards all calls to this
 * implementation.
 */
class WS2812b_DeviceImpl {
public:
  WS2812b_DeviceImpl(WS2812b_Device& parent, WS2812b_Params params, com::ComInterface* interface);

  ~WS2812b_DeviceImpl();

  const WS2812b_Params& getParams() const;

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data);

  void onResetSensorState();
  void onClearDataFlag();

private:
  WS2812b_Device& _parent;
  const WS2812b_Params _params;
};

} // namespace device

} // namespace eduart

