// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthSensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for depth-sensing devices (ToF, structured light, etc.).
 */

#pragma once

#include <functional>

#include "sensorring/device/IDevice.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Publisher.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class DepthSensor
 * @brief Public interface for any depth-sensing device.
 *
 * Users subscribe to depth measurements via subscribe(). The concrete sensor
 * implementation publishes measurements by calling publishMeasurement() from
 * the state machine thread.
 */
class SENSORRING_EXPORT DepthSensor : public virtual IDevice {
public:
  /// Measurement type produced by this sensor category.
  using MeasurementType = measurement::DepthMeasurement;

  virtual ~DepthSensor() = default;

  /**
   * @brief Subscribe to depth measurements from this sensor.
   * @param[in] callback Invoked with each new measurement.
   * @return RAII Subscription that auto-cancels on destruction.
   */
  subscription::Subscription subscribe(std::function<void(const measurement::DepthMeasurement&)> callback) { return _depth_publisher.subscribe(std::move(callback)); }

  /**
   * @brief Build a DepthMeasurement from internal state and publish to all subscribers.
   *
   * Called by the state machine after a successful measurement fetch.
   */
  virtual void publishMeasurement() = 0;

protected:
  subscription::Publisher<const measurement::DepthMeasurement&> _depth_publisher;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
