// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementManager.hpp
 * @author EduArt Robotik GmbH
 * @brief  The MeasurementManager is the main interface of the sensorring library
 * @date   2024-12-25
 */

#pragma once

#include <functional>
#include <memory>

#include "sensorring/MeasurementClient.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

// Forward declaration of implementation class
class SENSORRING_EXPORT MeasurementManagerImpl;

/**
 * @class MeasurementManager
 * @brief Meta class that handles the timing, triggering and processing of sensor measurements. Internally it runs a
 * looping state machine.
 */
class SENSORRING_EXPORT MeasurementManager {
public:
  /**
   * Constructor
   */
  MeasurementManager(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring);

  /**
   * Destructor
   */
  ~MeasurementManager() noexcept;

  /**
   * Run one processing cycle of the state machine worker
   * @return true on success
   */
  bool measureSome() noexcept;

  /**
   * Start running the state machine worker loop in a thread
   * @return true on success
   */
  bool startMeasuring() noexcept;

  /**
   * Stop the state machine worker loop and close the thread
   * @return true on success
   */
  bool stopMeasuring() noexcept;

  /**
   * Query if the measurement thread is currently running
   * @return true if the measurement thread is running
   */
  bool isMeasuring() noexcept;

  /**
   * Register an observer with the MeasurementManager object
   * @param[in] observer Observer that is registered and gets notified on future events
   * @throw std::runtime_error if Logger::log() throws
   */
  void registerClient(MeasurementClient* observer);

  /**
   * Unregister an observer with the MeasurementManager object
   * @param[in] observer Observer that is unregistered and will not be notified on future events
   * @throw std::runtime_error if Logger::log() throws
   */
  void unregisterClient(MeasurementClient* observer);

  /**
   * Get the health status of the state machine
   * @return Current manager state
   */
  ManagerState getManagerState() const noexcept;

  /**
   * Get the parameters with which the MeasurementManager was initialized
   * @return Initial parameter struct
   */
  ManagerParams getParams() const noexcept;

  /**
   * Get the SensorRing managed by this MeasurementManager.
   * @return Pointer to the managed SensorRing
   */
  ring::SensorRing* getSensorRing() const noexcept;

  /**
   * Queue an extra action that will be executed in the
   * extra actions slot of the internal state machine.
   * @param[in] action callable to be executed once in the next extra actions slot
   */
  void enqueueExtraAction(std::function<void()> action);

private:
  std::unique_ptr<MeasurementManagerImpl> _mm_impl;
};

} // namespace manager

} // namespace eduart