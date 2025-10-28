#pragma once

#include <memory>
#include <mutex>
#include "interface/ComInterface.hpp"
#include "interface/ComObserver.hpp"
#include "sensors/LedLight.hpp"
#include "sensors/ThermalSensor.hpp"
#include "sensors/TofSensor.hpp"
#include "utils/EnumerationInformation.hpp"

#include "Parameters.hpp"

namespace eduart {

namespace sensor {

class SensorBoard : public com::ComObserver {
public:
  SensorBoard(SensorBoardParams params, com::ComInterface* interface, std::size_t idx, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds);
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

  std::unique_ptr<TofSensor> _tof;
  std::unique_ptr<ThermalSensor> _thermal;
  std::unique_ptr<LedLight> _leds;

  mutable std::mutex _com_mutex;
  using LockGuard = std::lock_guard<std::mutex>;
};

} // namespace sensor

} // namespace eduart