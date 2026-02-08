#pragma once

#include <memory>
#include <mutex>

#include "device/hardware/htpa32/HTPA32_Device.hpp"
#include "device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "device/hardware/ws2812b/WS2812b_Device.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/interface/ComObserver.hpp"
#include "types/EnumerationInformation.hpp"

namespace eduart {

namespace device {

class SensorBoard : public com::ComObserver {
public:
  SensorBoard(SensorBoardParams params, com::ComInterface* interface, unsigned int idx, std::unique_ptr<VL53L8CX_Device> tof, std::unique_ptr<HTPA32_Device> thermal, std::unique_ptr<WS2812b_Device> leds);
  ~SensorBoard();

  bool isEnumerated() const;
  const EnumerationInformation& getEnumInfo() const;

  std::vector<BaseDevice*> getDevices() const;

  VL53L8CX_Device* getTof() const;
  HTPA32_Device* getThermal() const;
  WS2812b_Device* getLed() const;

  static void cmdReset(com::ComInterface* interface);
  static void cmdSetBrs(com::ComInterface* interface, bool enable);
  static void cmdEnumerateBoards(com::ComInterface* interface);

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  int _idx;
  com::ComInterface* _interface;
  const SensorBoardParams _params;
  EnumerationInformation _enum_info;

  std::vector<std::unique_ptr<device::BaseDevice> > _device_vec;

  mutable std::recursive_mutex _com_mutex;
  using LockGuard = std::lock_guard<std::recursive_mutex>;
};

} // namespace device

} // namespace eduart