// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   VL53L8CX_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for the VL53L8CX Time-of-Flight sensor device
 * @date   2025-02-11
 */

#pragma once

#include <chrono>
#include <future>
#include <memory>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/TofMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

class VL53L8CX_DeviceImpl;

/**
 * @class VL53L8CX_Device
 * @brief  Device wrapper for a VL53L8CX Time-of-Flight sensor on the sensorring bus.
 */
class SENSORRING_EXPORT VL53L8CX_Device : public BaseDevice {
public:
  /**
   * @brief Construct a new VL53L8CX device instance.
   * @param[in] params    Sensor configuration parameters.
   * @param[in] interface Communication interface used to talk to the device.
   * @param[in] idx       Index of the sensor on the bus.
   */
  VL53L8CX_Device(VL53L8CX_Params params, com::ComInterfaceID interface, unsigned int idx);
  /// Destructor
  ~VL53L8CX_Device();

  /**
   * @brief Get the sensor parameters used to configure this device.
   * @return Reference to the internal VL53L8CX parameter struct.
   */
  const VL53L8CX_Params& getParams() const;

  /**
   * @brief Get the most recent measurement and current sensor state.
   * @return Pair of latest Time-of-Flight measurement and associated sensor state.
   */
  std::pair<const measurement::TofMeasurement&, DeviceState> getLatestMeasurement() const;
  /**
   * @brief Get the most recent transformed measurement and current sensor state.
   * @return Pair of latest transformed Time-of-Flight measurement and associated sensor state.
   */
  std::pair<const measurement::TofMeasurement&, DeviceState> getLatestTransformedMeasurement() const;

  // std::future<bool> requestTofMeasurementAsync(std::chrono::milliseconds timeout);
  // std::future<bool> fetchTofMeasurementAsync(std::chrono::milliseconds timeout);
  /**
   * @brief Request Time-of-Flight measurements asynchronously on a set of devices.
   * @param[in] devices Vector of devices to trigger.
   * @param[in] timeout Maximum time to wait for completion.
   * @return Future resolving to true when all requests succeed.
   */
  static std::future<bool> requestTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout);
  /**
   * @brief Fetch Time-of-Flight measurements asynchronously from a set of devices.
   * @param[in] devices Vector of devices to read from.
   * @param[in] timeout Maximum time to wait for completion.
   * @return Future resolving to true when all fetches succeed.
   */
  static std::future<bool> fetchTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout);

private:
  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) override;

  friend class VL53L8CX_DeviceImpl;
  std::unique_ptr<VL53L8CX_DeviceImpl> _impl;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
