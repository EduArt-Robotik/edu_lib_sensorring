// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DeviceGroup.hpp
 * @author EduArt Robotik GmbH
 * @brief  Container for a group of devices with filtering and iteration by type.
 * @date   2025-02-08
 */

#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <vector>

#include "sensorring/platform/SensorringExport.hpp"

#include "IDevice.hpp"

namespace eduart {

namespace device {

/**
 * @class DeviceGroup
 * @brief Holds a set of IDevice pointers and provides type-filtered access and iteration.
 */
class SENSORRING_EXPORT DeviceGroup {

public:
  /**
   * @brief Constructs a group from the given device pointers.
   * @param[in] devices Pointers to devices to include in the group.
   */
  DeviceGroup(std::vector<device::IDevice*> devices);

  /// Destructor
  ~DeviceGroup() = default;

  /**
   * @brief Returns all devices in the group.
   * @return Vector of IDevice pointers (order preserved).
   */
  std::vector<device::IDevice*> getDevices() const;

  /**
   * @brief Returns the number of devices in the group.
   * @return Number of devices in the group.
   */
  unsigned int getDeviceCount() const;

  /**
   * @brief Waits until all futures are ready within the given timeout, then checks each result with a predicate.
   * @tparam Response Type of the future result.
   * @tparam Predicate Callable with signature bool(const Response&); if it returns false for any result, this returns false.
   * @param[in,out] futures Vector of futures to wait on (will be moved-from / consumed).
   * @param[in] timeout Maximum time to wait.
   * @param[in] success_predicate Called once per future result; all must return true for this to return true.
   * @return true if all futures became ready within the timeout and the predicate returned true for every result; false otherwise.
   */
  template <typename Response, typename Predicate> static bool waitForAll(std::vector<std::future<Response> >& futures, std::chrono::steady_clock::duration timeout, Predicate&& success_predicate) noexcept;

  /**
   * @brief Invokes callback once per device in the group.
   * @param[in] callback Callable invoked with each device pointer.
   */
  void invokeForEachDevice(std::function<void(device::IDevice*)> callback) const;

  /**
   * @brief Returns devices that are of type T (dynamic_cast).
   * @tparam T Device type derived from IDevice.
   * @return Vector of T* for devices that support the cast.
   */
  template <typename T> std::vector<T*> getDevicesOfType() const;

  /**
   * @brief Invokes callback for each device that is of type T.
   * @tparam T Device type derived from IDevice.
   * @param[in] callback Callable invoked with each T*.
   */
  template <typename T> void invokeForEachDeviceOfType(std::function<void(T*)> callback) const;

  /**
   * @brief Builds a DeviceGroup containing only devices of type T from the given list.
   * @tparam T Device type derived from IDevice.
   * @param[in] devices Source device list to filter.
   * @return DeviceGroup containing only the T* devices from devices.
   */
  template <typename T> static DeviceGroup createFromDevicesOfType(std::vector<device::IDevice*> devices);

private:
  std::vector<device::IDevice*> _devices;
};

// Template implementations
template <typename T> std::vector<T*> DeviceGroup::getDevicesOfType() const {
  std::vector<T*> devices;
  for (auto& device : _devices) {
    if (auto* cast = dynamic_cast<T*>(device)) {
      devices.push_back(cast);
    }
  }
  return devices;
}

template <typename T> void DeviceGroup::invokeForEachDeviceOfType(std::function<void(T*)> callback) const {
  for (auto& device : _devices) {
    if (auto* cast = dynamic_cast<T*>(device)) {
      callback(cast);
    }
  }
}

template <typename T> DeviceGroup DeviceGroup::createFromDevicesOfType(std::vector<device::IDevice*> devices) {
  std::vector<device::IDevice*> filtered;
  for (auto& device : devices) {
    if (dynamic_cast<T*>(device)) {
      filtered.push_back(device);
    }
  }
  return DeviceGroup(std::move(filtered));
}

template <typename Response, typename Predicate> bool DeviceGroup::waitForAll(std::vector<std::future<Response> >& futures, std::chrono::steady_clock::duration timeout, Predicate&& success_predicate) noexcept {
  if (futures.empty())
    return true;
  const auto deadline = std::chrono::steady_clock::now() + timeout;

  // First, wait for all futures to become ready (or until timeout).
  for (auto& fut : futures) {
    if (fut.wait_until(deadline) != std::future_status::ready) {
      return false; // At least one future did not complete in time.
    }
  }

  // Then, consume results and apply the predicate.
  bool all_ok = true;
  for (auto& fut : futures) {
    if (!success_predicate(fut.get())) {
      all_ok = false;
    }
  }
  return all_ok;
}

} // namespace device

} // namespace eduart