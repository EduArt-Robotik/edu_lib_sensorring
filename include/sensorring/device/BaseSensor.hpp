// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   BaseSensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base class for sensors using a communication interface.
 * @date   2025-02-10
 */

#pragma once

#include <cstdint>
#include <future>
#include <mutex>
#include <optional>
#include <vector>

#include "sensorring/device/DeviceState.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Matrix3.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {
// Forward declaration
namespace com {
class ComInterface;
}

namespace device {

/**
 * @class BaseSensor
 * @brief Abstract base class for sensors.
 *
 * Provides state tracking, pose handling and notification hooks for derived sensors.
 */
class SENSORRING_EXPORT BaseSensor {
public:
  /**
   * @brief Construct a base sensor and register it with the communication interface.
   * @param[in] interface Communication interface used to exchange messages with the sensor.
   * @param[in] target Endpoint on the interface this sensor listens to.
   * @param[in] idx Index of the sensor instance.
   * @param[in] enable Initial enabled state of the sensor.
   */
  BaseSensor(com::ComInterface* interface, com::ComEndpoint target, unsigned int idx, bool enable);

  /// Destructor
  virtual ~BaseSensor();

  /**
   * @brief Get the index of this sensor instance.
   * @return Sensor index.
   */
  unsigned int getIdx() const;

  /**
   * @brief Query if the sensor is currently enabled.
   * @return true if the sensor is enabled.
   */
  bool getEnable() const;

  /**
   * @brief Enable or disable the sensor.
   * @param[in] enable New enabled state. Set to true to enable the sensor, false to disable it.
   */
  void setEnable(bool enable);

  /**
   * @brief Set the pose of the sensor in the common coordinate frame.
   * @param[in] translation Translation of the sensor origin.
   * @param[in] rotation Rotation as Euler angles in degrees.
   */
  void setPose(math::Vector3 translation, math::Vector3 rotation);

  /**
   * @brief Reset error state and internal flags to nominal values.
   *
   * Calls the sensor-specific hook onResetSensorState().
   */
  void resetSensorState();

  /**
   * @brief Clear state for the next measurement cycle. Calls the sensor-specific hook onClearDataFlag().
   */
  void clearDataFlag();

  /**
   * @brief Start a measurement-wait cycle and return a future that will be set when the measurement is ready.
   * @return Future that will hold true when the measurement has been fetched successfully.
   */
  std::future<bool> beginMeasurementWait();

  /**
   * @brief Start a data-available-wait cycle and return a future that will be set when new data is available.
   * @return Future that will hold true when new data has been signalled as available.
   */
  std::future<bool> beginDataAvailableWait();

protected:
  /**
   * @brief Set the result of the current measurement-wait cycle. Called from derived callbacks when ready.
   * @param[in] success true if measurement was fetched successfully.
   */
  void setMeasurementReady(bool success);

  /**
   * @brief Set the result of the current data-available-wait cycle. Called from derived callbacks when "data available" is received.
   * @param[in] success true if data available was signalled successfully.
   */
  void setDataAvailableReady(bool success);

protected:
  /**
   * @brief Sensor-specific hook invoked from resetSensorState().
   *
   * Implementations should reset any additional state related to errors or measurements.
   */
  virtual void onResetSensorState() {};

  /**
   * @brief Sensor-specific hook invoked from clearDataFlag().
   *
   * Implementations should clear any additional flags related to buffered data.
   */
  virtual void onClearDataFlag() {};

  /**
   * @brief Handle an incoming communication message for this sensor.
   * @param[in] source Endpoint that sent the message.
   * @param[in] data   Message payload.
   */
  virtual void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<std::uint8_t>& data) = 0;

  /// Index of this sensor instance within its group.
  unsigned int _idx;
  /// Current health state of the sensor.
  DeviceState _error;
  /// Communication interface used to talk to the sensor.
  com::ComInterface* _interface;

  /// Translation of the sensor origin in the common coordinate frame.
  math::Vector3 _translation;
  /// Rotation of the sensor expressed as Euler angles in degrees.
  math::Vector3 _rotation;
  /// Rotation matrix derived from the Euler angles.
  math::Matrix3 _rot_m;

  /// Flag indicating whether the sensor is enabled.
  bool _enable_flag;
  /// Mutex protecting sensor state mutations.
  /// @note Locking order: acquire _state_mutex before _promise_mutex.
  mutable std::mutex _state_mutex;

  /// Promise for the current measurement-wait cycle; set by callback, consumed by wait + get().
  std::optional<std::promise<bool> > _measurement_promise;
  /// Promise for the current data-available-wait cycle; set by callback when "data available" is received.
  std::optional<std::promise<bool> > _data_available_promise;
  /// Protects promise lifecycles (create in batch thread, set in callback thread).
  /// @note Locking order: acquire _state_mutex before _promise_mutex.
  std::mutex _promise_mutex;

  /// RAII subscription to the communication interface.
  subscription::Subscription _com_subscription;
};

} // namespace device

} // namespace sensorring

} // namespace eduart