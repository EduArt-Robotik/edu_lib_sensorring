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

#include "sensorring/device/thermal/ThermalSensor.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
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
class SENSORRING_EXPORT HTPA32_Device : public ThermalSensor {
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
  const HTPA32_Params& getParams() const override;

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
  bool stopCalibration() override;

  /**
   * @brief Start a thermal calibration over a sliding window of frames.
   * @param[in] window Number of frames to average for calibration.
   * @return true on success.
   */
  bool startCalibration(unsigned int window) override;

  /**
   * @brief Request thermal measurements asynchronously on a set of devices.
   *
   * Sends a single broadcast MEASUREMENT_REQUEST per communication interface.
   * @param[in] devices Vector of devices to trigger.
   * @param[in] timeout Maximum time to wait (fire-and-forget; timeout is unused).
   * @return Future resolving to true when the request has been sent.
   */
  static std::future<bool> requestMeasurementAsync(const std::vector<HTPA32_Device*>& devices, std::chrono::milliseconds timeout);
  /**
   * @brief Request a thermal measurement asynchronously from this single device.
   *
   * Sends a direct MEASUREMENT_REQUEST to this board. Fire-and-forget.
   * @param[in] timeout Maximum time to wait (unused; returns immediately).
   * @return Future resolving to true when the request has been sent.
   */
  std::future<bool> requestMeasurementAsync(std::chrono::milliseconds timeout);
  /**
   * @brief Fetch the thermal measurement asynchronously from this device.
   *
   * Sends a direct MEASUREMENT_TRANSMISSION_REQUEST to this single device and waits
   * for the response. Must be called sequentially, one device at a time.
   * @param[in] timeout Maximum time to wait for data transmission.
   * @return Future resolving to true when the measurement data has been received.
   */
  std::future<bool> fetchMeasurementAsync(std::chrono::milliseconds timeout);

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
