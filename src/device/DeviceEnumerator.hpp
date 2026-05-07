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
#include "sensorring/device/types/EnumerationInformation.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

namespace device {

class DeviceEnumerator {
public:
  DeviceEnumerator(com::ComInterface* interface);

  ~DeviceEnumerator();

  void startEnumeration();

  std::vector<device::EnumerationInformation> getResult();

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

private:
  using Mutex     = std::mutex;
  using LockGuard = std::lock_guard<Mutex>;

  mutable Mutex _enumeration_mutex;

  com::ComInterface* _interface;
  std::vector<device::EnumerationInformation> _enumeration_vec;

  subscription::Subscription _com_subscription;
};

} // namespace device

} // namespace sensorring

} // namespace eduart