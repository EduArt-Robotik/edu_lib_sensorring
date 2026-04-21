// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ThermalSensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for thermal-sensing devices.
 */

#pragma once

#include <functional>

#include "sensorring/device/IDevice.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Publisher.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class ThermalSensor
 * @brief Public interface for any thermal-sensing device.
 *
 * Users subscribe to thermal measurements via subscribe(). The concrete sensor
 * implementation publishes measurements by calling publishMeasurement() from
 * the state machine thread.
 */
class SENSORRING_EXPORT ThermalSensor : public virtual IDevice {
public:
  /// Measurement type produced by this sensor category.
  using MeasurementType = measurement::ThermalMeasurement;

  virtual ~ThermalSensor() = default;

  /**
   * @brief Subscribe to thermal measurements from this sensor.
   * @param[in] callback Invoked with each new measurement.
   * @return RAII Subscription that auto-cancels on destruction.
   */
  subscription::Subscription subscribe(std::function<void(const measurement::ThermalMeasurement&)> callback) { return _thermal_publisher.subscribe(std::move(callback)); }

  /**
   * @brief Build a ThermalMeasurement from internal state and publish to all subscribers.
   *
   * Called by the state machine after a successful measurement fetch.
   */
  virtual void publishMeasurement() = 0;

  /**
   * @brief Start a thermal calibration over a sliding window of frames.
   * @param[in] window Number of frames to average for calibration.
   * @return true on success.
   */
  virtual bool startCalibration(unsigned int window) = 0;

  /**
   * @brief Stop any ongoing thermal calibration sequence.
   * @return true on success.
   */
  virtual bool stopCalibration() = 0;

protected:
  subscription::Publisher<const measurement::ThermalMeasurement&> _thermal_publisher;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
