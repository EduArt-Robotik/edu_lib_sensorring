#pragma once

#include <memory>
#include <mutex>

#include "device/LedLight.hpp"
#include "device/ThermalSensor.hpp"
#include "device/TofSensor.hpp"
#include "interface/ComInterface.hpp"
#include "interface/ComObserver.hpp"
#include "sensorring/Parameter.hpp"
#include "types/EnumerationInformation.hpp"

namespace eduart {

namespace device {

class SensorBoard : public com::ComObserver {
public:
  SensorBoard(SensorBoardParams params, com::ComInterface* interface, unsigned int idx, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds);
  ~SensorBoard();

  bool isEnumerated() const;
  const EnumerationInformation& getEnumInfo() const;

  TofSensor* getTof() const;
  ThermalSensor* getThermal() const;
  LedLight* getLed() const;

  static void cmdReset(com::ComInterface* interface);
  static void cmdSetBrs(com::ComInterface* interface, bool enable);
  static void cmdEnumerateBoards(com::ComInterface* interface);

  void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  int _idx;
  com::ComInterface* _interface;
  const SensorBoardParams _params;
  EnumerationInformation _enum_info;

  std::vector<std::unique_ptr<device::IDevice> > _device_vec;

  mutable std::mutex _com_mutex;
  using LockGuard = std::lock_guard<std::mutex>;
};

} // namespace device

} // namespace eduart