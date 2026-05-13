// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   TMF8829_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for the TMF8829 Time-of-Flight sensor device
 * @date   2025-02-11
 */

#pragma once

#include <chrono>
#include <future>
#include <memory>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

class TMF8829_DeviceImpl;

/**
 * @class TMF8829_Device
 * @brief  Device wrapper for a TMF8829 Time-of-Flight sensor on the sensorring bus.
 */
class SENSORRING_EXPORT TMF8829_Device : public BaseDevice, public DepthSensor {
public:
  /**
   * @brief Construct a new TMF8829 device instance.
   * @param[in] params    Sensor configuration parameters.
   * @param[in] interface Communication interface used to talk to the device.
   * @param[in] idx       Index of the sensor on the bus.
   */
  TMF8829_Device(TMF8829_Params params, com::ComInterfaceID interface, unsigned int idx);
  /// Destructor
  ~TMF8829_Device();

  /**
   * @brief Get the sensor parameters used to configure this device.
   * @return Reference to the internal TMF8829 parameter struct.
   */
  const TMF8829_Params& getParams() const;

  /**
   * @brief Get the resolution mode of the sensor.
   * @return Current resolution mode. Valid values are 0 to 8.
   */
  int getResolutionMode();

  /**
   * @brief Set the resolution mode of the sensor.
   * @param[in] mode Resolution mode to set. Valid values are 0 to 8.
   * @return Return true if setting the resolution mode was successful.
   */
  bool setResolutionMode(std::uint8_t mode);

  /**
   * @brief Get the most recent measurement and current sensor state.
   * @return Pair of latest depth measurement and associated sensor state.
   */
  std::pair<const measurement::DepthMeasurement&, DeviceState> getLatestMeasurement() const;

  /**
   * @brief Request Time-of-Flight measurements asynchronously on a set of devices.
   *
   * Sends a single broadcast MEASUREMENT_REQUEST per communication interface.
   * @param[in] devices Vector of devices to trigger.
   * @param[in] timeout Maximum time to wait for all devices to acknowledge readiness.
   * @return Future resolving to true when all requests succeed.
   */
  static std::future<bool> requestMeasurementAsync(const std::vector<TMF8829_Device*>& devices, std::chrono::milliseconds timeout);
  /**
   * @brief Fetch the Time-of-Flight measurement asynchronously from this device.
   *
   * Sends a direct MEASUREMENT_TRANSMISSION_REQUEST to this single device and waits
   * for the response. Must be called sequentially, one device at a time.
   * @param[in] timeout Maximum time to wait for data transmission.
   * @return Future resolving to true when the measurement data has been received.
   */
  std::future<bool> fetchMeasurementAsync(std::chrono::milliseconds timeout);

  /**
   * @brief Build a DepthMeasurement from internal state and publish to subscribers.
   */
  void publishMeasurement() override;

private:
  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) override;

  friend class TMF8829_DeviceImpl;
  std::unique_ptr<TMF8829_DeviceImpl> _impl;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
