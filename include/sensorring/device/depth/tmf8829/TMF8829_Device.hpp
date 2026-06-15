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
class SENSORRING_EXPORT TMF8829_Device : public DepthSensor {
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
   * @brief Set the resolution mode of the sensor.
   * @param[in] mode Resolution mode to set.
   * @return Return true if setting the resolution mode was successful.
   */
  bool setResolutionMode(ResolutionMode mode);

  /**
   * @brief Get the resolution mode of the sensor.
   * @param[out] mode Reference to store the current resolution mode.
   * @return Return true if reading the current resolution mode was successful.
   */
  bool getResolutionMode(ResolutionMode& mode);

  /**
   * @brief Set the iterations setting of the sensor in kilo iteration per measurement.
   * @param[in] k_iterations Iterations setting to set. Refer to the TMF8829 datasheet for more details about this setting.
   * @return Return true if setting the iterations setting was successful.
   */
  bool setIterationsSetting(std::uint16_t k_iterations);

  /**
   * @brief Get the iterations setting of the sensor in kilo iteration per measurement.
   * @param[out] k_iterations Reference to store the current iterations setting. Refer to the TMF8829 datasheet for more details about this setting.
   * @return Return true if reading the current iterations setting was successful.
   */
  bool getIterationsSetting(std::uint16_t& k_iterations);

  /**
   * @brief Set the result format of the sensor.
   * @param[in] format Result format to set.
   * @return Return true if setting the result format was successful.
   */
  bool setResultFormat(TMF8829_ResultFormat format);

  /**
   * @brief Get the result format of the sensor.
   * @param[out] format Reference to store the current result format.
   * @return Return true if reading the current result format was successful.
   */
  bool getResultFormat(TMF8829_ResultFormat& format);

  /**
   * @brief Enable or disable full noise data in measurement results.
   * @param[in] full_noise Set to true to include full noise data in results.
   * @return Return true if setting the full noise flag was successful.
   */
  bool setResultFullNoise(bool full_noise);

  /**
   * @brief Get whether full noise data is included in measurement results.
   * @param[out] full_noise Reference to store the current full noise flag.
   * @return Return true if reading the current full noise flag was successful.
   */
  bool getResultFullNoise(bool& full_noise);

  /**
   * @brief Enable or disable crosstalk data in measurement results.
   * @param[in] xtalk Set to true to include crosstalk data in results.
   * @return Return true if setting the crosstalk flag was successful.
   */
  bool setResultXtalk(bool xtalk);

  /**
   * @brief Get whether crosstalk data is included in measurement results.
   * @param[out] xtalk Reference to store the current crosstalk flag.
   * @return Return true if reading the current crosstalk flag was successful.
   */
  bool getResultXtalk(bool& xtalk);

  /**
   * @brief Enable or disable noise strength data in measurement results.
   * @param[in] noise_strength Set to true to include noise strength in results.
   * @return Return true if setting the noise strength flag was successful.
   */
  bool setResultNoiseStrength(bool noise_strength);

  /**
   * @brief Get whether noise strength data is included in measurement results.
   * @param[out] noise_strength Reference to store the current noise strength flag.
   * @return Return true if reading the current noise strength flag was successful.
   */
  bool getResultNoiseStrength(bool& noise_strength);

  /**
   * @brief Enable or disable signal strength data in measurement results.
   * @param[in] signal_strength Set to true to include signal strength in results.
   * @return Return true if setting the signal strength flag was successful.
   */
  bool setResultSignalStrength(bool signal_strength);

  /**
   * @brief Get whether signal strength data is included in measurement results.
   * @param[out] signal_strength Reference to store the current signal strength flag.
   * @return Return true if reading the current signal strength flag was successful.
   */
  bool getResultSignalStrength(bool& signal_strength);

  /**
   * @brief Set the maximum number of peaks reported per pixel in measurement results.
   * @param[in] nr_of_peaks Maximum number of peaks to report.
   * @return Return true if setting the number of peaks was successful.
   */
  bool setResultNrOfPeaks(std::uint8_t nr_of_peaks);

  /**
   * @brief Get the maximum number of peaks reported per pixel in measurement results.
   * @param[out] nr_of_peaks Reference to store the current maximum number of peaks.
   * @return Return true if reading the current number of peaks was successful.
   */
  bool getResultNrOfPeaks(std::uint8_t& nr_of_peaks);

  /**
   * @brief Re-apply runtime configuration after a board reset.
   * @return true on success.
   */
  bool configure() override;

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
