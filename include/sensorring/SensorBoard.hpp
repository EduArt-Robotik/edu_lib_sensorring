// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoard.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstraction of a single sensor board
 * @date   2025-02-19
 */

#pragma once

#include <memory>
#include <mutex>

#include "sensorring/SensorBoardParams.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/EnumerationInformation.hpp"
#include "sensorring/interface/ComObserver.hpp"

namespace eduart {

namespace com {
class ComInterface;
}

namespace device {

struct SensorBoard : com::ComObserver, IDevice {
public:
  SensorBoard(SensorBoardParams params, com::ComInterface* interface, unsigned int idx, std::vector<std::unique_ptr<BaseDevice> > devices);
  ~SensorBoard();

  bool isEnumerated() const;
  const EnumerationInformation& getEnumInfo() const;

  std::vector<BaseDevice*> getDevices() const;

  static bool resetBoards();

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