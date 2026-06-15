// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Sensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base class for all sensor devices, adding measurement synchronisation to Device.
 * @date   2026-06-15
 */

#pragma once

#include <future>
#include <mutex>
#include <optional>

#include "sensorring/device/Device.hpp"
#include "sensorring/device/types/DeviceState.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class Sensor
 * @brief Base class for all sensor devices in the SensorRing.
 *
 * Extends Device with promise-based measurement synchronisation and
 * sensor state management used by the state machine scheduler.
 */
class SENSORRING_EXPORT Sensor : public Device {
public:
  /**
   * @brief Construct the sensor with identity, communication link and enable flag.
   * @param[in] id        Device identifier.
   * @param[in] interface Communication interface.
   * @param[in] target    Communication endpoint this device listens to.
   * @param[in] enable    Whether the device starts enabled.
   */
  Sensor(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable);

  ~Sensor() override;

  // -- state / measurement synchronisation --

  /**
   * @brief Begin an asynchronous wait for the next measurement trigger.
   * @return Future that resolves to @c true when the measurement trigger fires, or @c false on shutdown.
   */
  std::future<bool> beginMeasurementWait();

  /**
   * @brief Satisfy the pending measurement-wait future.
   * @param[in] success @c true if the measurement succeeded, @c false on error.
   */
  void setMeasurementReady(bool success);

  /**
   * @brief Begin an asynchronous wait for new data to become available.
   * @return Future that resolves to @c true when data is available, or @c false on shutdown.
   */
  std::future<bool> beginDataAvailableWait();

  /**
   * @brief Satisfy the pending data-available-wait future.
   * @param[in] success @c true if data is ready, @c false on error.
   */
  void setDataAvailableReady(bool success);

  /// @brief Reset the device to its initialised state, clearing all error flags.
  void resetSensorState();

  /// @brief Clear the data-available flag so the device can accept the next measurement cycle.
  void clearDataFlag();

protected:
  virtual void onResetSensorState() {}
  virtual void onClearDataFlag() {}

  DeviceState _state;
  mutable std::mutex _state_mutex;

  std::mutex _promise_mutex;
  std::optional<std::promise<bool> > _data_available_promise;
  std::optional<std::promise<bool> > _measurement_promise;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
