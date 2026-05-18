// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   BoardEnumerator.hpp
 * @author EduArt Robotik GmbH
 * @brief  Enumerator for boards on a sensor bus.
 * @date   2026-02-24
 */

#pragma once

#include <mutex>
#include <vector>

#include "interface/ComInterface.hpp"
#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

namespace board {

class BoardEnumerator {
public:
  BoardEnumerator(com::ComInterface* interface);

  ~BoardEnumerator();

  void startEnumeration();

  std::vector<EnumerationInformation> getResult();

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

private:
  using Mutex     = std::mutex;
  using LockGuard = std::lock_guard<Mutex>;

  mutable Mutex _enumeration_mutex;

  com::ComInterface* _interface;
  std::vector<EnumerationInformation> _enumeration_vec;

  subscription::Subscription _com_subscription;
};

} // namespace board

} // namespace sensorring

} // namespace eduart