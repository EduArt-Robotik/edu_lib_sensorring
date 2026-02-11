// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   BaseSensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base class for sensors using a communication interface.
 * @date   2026-02-10
 */

#pragma once

#include <atomic>
#include <mutex>

#include "interface/ComInterface.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Matrix3.hpp"

namespace eduart {

namespace device {

/**
 * @enum SensorState
 * @brief State of a sensor instance.
 */
enum class SENSORRING_EXPORT SensorState {
  SensorInit,
  SensorOK,
  ReceiveError
};

/**
 * @class BaseSensor
 * @brief Abstract base class for sensors.
 *
 * Provides state tracking, pose handling and notification hooks for derived sensors.
 */
class BaseSensor : public com::ComObserver {
public:
  /**
   * @brief Construct a base sensor and register it with the communication interface.
   * @param[in] interface Communication interface used to exchange messages with the sensor.
   * @param[in] target Endpoint on the interface this sensor listens to.
   * @param[in] idx Index of the sensor instance.
   * @param[in] enable Initial enabled state of the sensor.
   */
  BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable);
  /// Destructor
  ~BaseSensor();

  /**
   * @brief Get the index of this sensor instance.
   * @return Sensor index.
   */
  std::size_t getIdx() const;

  /**
   * @brief Check if a new measurement is ready for consumption.
   * @return true if a new measurement has been processed and is ready.
   */
  bool gotNewData() const;

  /**
   * @brief Check if new raw data from the sensor is available.
   * @return true if new data has been received into the internal buffer.
   */
  bool newDataAvailable() const;

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
   * @brief Clear flags indicating the presence of new data.
   *
   * Calls the sensor-specific hook onClearDataFlag().
   */
  void clearDataFlag();

protected:
  /**
   * @brief Sensor-specific hook invoked from resetSensorState().
   *
   * Implementations should reset any additional state related to errors or measurements.
   */
  virtual void onResetSensorState() = 0;

  /**
   * @brief Sensor-specific hook invoked from clearDataFlag().
   *
   * Implementations should clear any additional flags related to buffered data.
   */
  virtual void onClearDataFlag() = 0;

  /// Index of this sensor instance within its group.
  std::size_t _idx;
  /// Current health state of the sensor.
  SensorState _error;
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
  /// Flag indicating that new data is available from the sensor.
  std::atomic<bool> _new_data_available_flag;
  /// Flag indicating that new data has been written into the internal buffer.
  std::atomic<bool> _new_data_in_buffer_flag;
  /// Flag indicating that a new measurement has been fully processed and is ready.
  std::atomic<bool> _new_measurement_ready_flag;
  /// Mutex protecting sensor state mutations.
  mutable std::mutex _state_mutex;
};

} // namespace device

} // namespace eduart