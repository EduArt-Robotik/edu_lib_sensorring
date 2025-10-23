#pragma once

#include <array>
#include <memory>
#include <mutex>

#include "boardmanager/SensorBoardManager.hpp"
#include "interface/ComInterface.hpp"
#include "interface/ComObserver.hpp"
#include "sensors/LedLight.hpp"
#include "sensors/ThermalSensor.hpp"
#include "sensors/TofSensor.hpp"
#include "utils/Version.hpp"

#include "Parameters.hpp"

namespace eduart {

namespace sensor {

class SensorBoard : public com::ComObserver {
public:
  SensorBoard(SensorBoardParams params, com::ComInterface* interface, std::size_t idx, std::unique_ptr<TofSensor> tof, std::unique_ptr<ThermalSensor> thermal, std::unique_ptr<LedLight> leds);
  ~SensorBoard();

  SensorBoardType getType() const;
  Version getFwRevision() const;
  CommitHash getFwHash() const;

  TofSensor* getTof() const;
  ThermalSensor* getThermal() const;
  LedLight* getLed() const;

  void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  int _idx;
  com::ComInterface* _interface;
  const SensorBoardParams _params;

  SensorBoardType _board_type;
  Version _fw_rev;
  CommitHash _fw_hash;

  std::unique_ptr<TofSensor> _tof;
  std::unique_ptr<ThermalSensor> _thermal;
  std::unique_ptr<LedLight> _leds;

  mutable std::mutex _com_mutex;
  using LockGuard = std::lock_guard<std::mutex>;
};

} // namespace sensor

} // namespace eduart