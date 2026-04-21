// Copyright (c) 2026 EduArt Robotik GmbH

#pragma once

#include <functional>
#include <mutex>
#include <vector>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class IDevice
 * @brief Thin base interface for all concrete devices in the SensorRing.
 *
 * Provides a common polymorphic base and a thread-safe per-device action queue.
 * Any device can enqueue self-contained actions (e.g. actuator commands) that the
 * state machine will drain and execute during the device_actions cycle.
 */
struct SENSORRING_EXPORT IDevice {
  virtual ~IDevice() = default;

  /**
   * @brief Enqueue a self-contained action to be executed by the state machine.
   * @param[in] action Callable executed once during the next device_actions slot.
   *                   Should be non-blocking and exception-safe.
   */
  void enqueueAction(std::function<void()> action) {
    std::lock_guard<std::mutex> lock(_action_mutex);
    _pending_actions.push_back(std::move(action));
  }

  /**
   * @brief Atomically drain and return all pending actions.
   * @return Vector of actions to execute. Empty if no actions were pending.
   */
  std::vector<std::function<void()> > drainActions() {
    std::lock_guard<std::mutex> lock(_action_mutex);
    std::vector<std::function<void()> > actions;
    actions.swap(_pending_actions);
    return actions;
  }

private:
  std::mutex _action_mutex;
  std::vector<std::function<void()> > _pending_actions;
};

} // namespace device

} // namespace sensorring

} // namespace eduart