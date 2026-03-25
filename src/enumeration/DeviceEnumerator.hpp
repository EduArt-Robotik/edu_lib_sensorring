// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DeviceEnumerator.hpp
 * @author EduArt Robotik GmbH
 * @brief  Enumerator for devices on a sensor board.
 * @date   2026-02-24
 */

#pragma once

#include <mutex>
#include <vector>

#include "interface/ComInterface.hpp"
#include "sensorring/enumeration/EnumerationInformation.hpp"
#include "sensorring/interface/ComObserver.hpp"

namespace eduart {

namespace com {
class ComInterface;
}

namespace device {

class DeviceEnumerator : public com::ComObserver {
public:
  DeviceEnumerator(com::ComInterface* interface);

  ~DeviceEnumerator();

  void startEnumeration();

  std::vector<device::EnumerationInformation> getResult();

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  using Mutex     = std::mutex;
  using LockGuard = std::lock_guard<Mutex>;

  mutable Mutex _enumeration_mutex;

  com::ComInterface* _interface;
  std::vector<device::EnumerationInformation> _enumeration_vec;
};

} // namespace device

} // namespace eduart