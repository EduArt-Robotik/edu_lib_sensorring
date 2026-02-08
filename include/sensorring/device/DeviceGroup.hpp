// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   DeviceGroup.hpp
 * @author EduArt Robotik GmbH
 * @brief  Container for a group of devices with filtering and iteration by type.
 * @date   2025-02-08
 */

#pragma once

#include "BaseDevice.hpp"

namespace eduart {

namespace device {

/**
 * @class DeviceGroup
 * @brief Holds a set of BaseDevice pointers and provides type-filtered access and iteration.
 */
class DeviceGroup {

public:
  /**
   * @brief Constructs a group from the given device pointers.
   * @param[in] devices Pointers to devices to include in the group.
   */
  DeviceGroup(std::vector<device::BaseDevice*> devices);

  /// Destructor
  ~DeviceGroup() = default;

  /**
   * @brief Returns all devices in the group.
   * @return Vector of BaseDevice pointers (order preserved).
   */
  std::vector<device::BaseDevice*> getDevices() const;

  /**
   * @brief Invokes callback once per device in the group.
   * @param[in] callback Callable invoked with each device pointer.
   */
  void invokeForEachDevice(std::function<void(device::BaseDevice*)> callback) const;

  /**
   * @brief Returns devices that are of type T (dynamic_cast).
   * @tparam T Device type derived from BaseDevice.
   * @return Vector of T* for devices that support the cast.
   */
  template <typename T> std::vector<T*> getDevicesOfType() const;

  /**
   * @brief Invokes callback for each device that is of type T.
   * @tparam T Device type derived from BaseDevice.
   * @param[in] callback Callable invoked with each T*.
   */
  template <typename T> void invokeForEachDeviceOfType(std::function<void(T*)> callback) const;

  /**
   * @brief Builds a DeviceGroup containing only devices of type T from the given list.
   * @tparam T Device type derived from BaseDevice.
   * @param[in] devices Source device list to filter.
   * @return DeviceGroup containing only the T* devices from devices.
   */
  template <typename T> static DeviceGroup createFromDevicesOfType(std::vector<device::BaseDevice*> devices);

private:
  std::vector<device::BaseDevice*> _devices;
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

template <typename T> DeviceGroup DeviceGroup::createFromDevicesOfType(std::vector<device::BaseDevice*> devices) {
  std::vector<device::BaseDevice*> filtered;
  for (auto& device : devices) {
    if (dynamic_cast<T*>(device)) {
      filtered.push_back(device);
    }
  }
  return DeviceGroup(filtered);
}

} // namespace device

} // namespace eduart