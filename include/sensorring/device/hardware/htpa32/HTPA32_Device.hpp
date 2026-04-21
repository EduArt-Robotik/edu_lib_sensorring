// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   HTPA32_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for the HTPA32 thermal sensor device
 * @date   2025-02-11
 */

#pragma once

#include <chrono>
#include <future>
#include <memory>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/ThermalSensor.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

class HTPA32_DeviceImpl;

/**
 * @class HTPA32_Device
 * @brief  Device wrapper for an HTPA32 thermal sensor on the sensorring bus.
 */
class SENSORRING_EXPORT HTPA32_Device : public BaseDevice, public ThermalSensor {
public:
  /**
   * @brief Construct a new HTPA32 device instance.
   * @param[in] params   Sensor configuration parameters.
   * @param[in] interface Communication interface used to talk to the device.
   * @param[in] idx      Index of the sensor on the bus.
   */
  HTPA32_Device(HTPA32_Params params, com::ComInterfaceID interface, unsigned int idx);

  /// Destructor
  ~HTPA32_Device();

  /**
   * @brief Get the sensor parameters used to configure this device.
   * @return Reference to the internal HTPA32 parameter struct.
   */
  const HTPA32_Params& getParams() const;


  /**
   * @brief Request the EEPROM content asynchronously.
   * @param[in] timeout Maximum time to wait for completion.
   * @return Future resolving to true on success.
   */
  std::future<bool> getEepromAsync(std::chrono::milliseconds timeout);
  /**
   * @brief Stop any ongoing thermal calibration sequence.
   * @return true on success.
   */
  bool stopCalibration();
  /**
   * @brief Start a thermal calibration over a sliding window of frames.
   * @param[in] window Number of frames to average for calibration.
   * @return true on success.
   */
  bool startCalibration(unsigned int window);
  /**
   * @brief Get the latest thermal measurement and current sensor state.
   * @return Pair of latest thermal measurement and associated sensor state.
   */
  std::pair<const measurement::ThermalMeasurement&, DeviceState> getLatestMeasurement() const;

  // std::future<bool> requestThermalMeasurementAsync(std::chrono::milliseconds timeout);
  // std::future<bool> fetchThermalMeasurementAsync(std::chrono::milliseconds timeout);
  /**
   * @brief Request thermal measurements asynchronously on a set of devices.
   * @param[in] devices Vector of devices to trigger.
   * @param[in] timeout Maximum time to wait for completion.
   * @return Future resolving to true when all requests succeed.
   */
  static std::future<bool> requestThermalMeasurementAsync(const std::vector<HTPA32_Device*>& devices, std::chrono::milliseconds timeout);
  /**
   * @brief Fetch thermal measurements asynchronously from a set of devices.
   * @param[in] devices Vector of devices to read from.
   * @param[in] timeout Maximum time to wait for completion.
   * @return Future resolving to true when all fetches succeed.
   */
  static std::future<bool> fetchThermalMeasurementAsync(const std::vector<HTPA32_Device*>& devices, std::chrono::milliseconds timeout);

  /**
   * @brief Build a ThermalMeasurement from internal state and publish to subscribers.
   */
  void publishMeasurement() override;

private:
  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) override;

  void onClearDataFlag() override;

  friend class HTPA32_DeviceImpl;
  std::unique_ptr<HTPA32_DeviceImpl> _impl;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
