// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   MeasurementManager.hpp
 * @author EduArt Robotik GmbH
 * @brief  The MeasurementManager is the main interface of the sensorring library
 * @date   2024-12-25
 */

#pragma once

#include <functional>
#include <memory>

#include "sensorring/SensorRing.hpp"
#include "sensorring/device/DeviceGroup.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/manager/ManagerParams.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/Subscription.hpp"
#include "sensorring/types/SubscriberToken.hpp"

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
   * @brief Construct the manager with parameters and owned SensorRing.
   * @param[in] params Manager configuration.
   * @param[in] sensor_ring SensorRing instance to manage (ownership transferred).
   */
  MeasurementManager(ManagerParams params, std::unique_ptr<ring::SensorRing> sensor_ring);

  /// Destructor
  ~MeasurementManager() noexcept;

  /**
   * @brief Run one processing cycle of the state machine worker.
   * @return true on success.
   */
  bool measureSome() noexcept;

  /**
   * @brief Start the state machine worker loop in a dedicated thread.
   * @return true on success.
   */
  bool startMeasuring() noexcept;

  /**
   * @brief Stop the state machine worker loop and join the thread.
   * @return true on success.
   */
  bool stopMeasuring() noexcept;

  /**
   * @brief Report whether the measurement worker thread is running.
   * @return true if the measurement thread is running.
   */
  bool isMeasuring() noexcept;

  /**
   * @brief Subscribe to state changes; callback is invoked when the state changes.
   * @param[in] callback Invoked with the updated ManagerState.
   * @return RAII Subscription that auto-cancels on destruction.
   */
  Subscription subscribeToStateChanges(std::function<void(const ManagerState state)> callback);

  /**
   * @brief Subscribe to device group updates; callback is invoked when the group is updated.
   * @param[in] key Device group to subscribe to.
   * @param[in] callback Invoked with the updated DeviceGroup.
   * @return RAII Subscription that auto-cancels on destruction.
   */
  Subscription subscribeToDeviceGroup(device::DeviceType key, std::function<void(const device::DeviceGroup&)> callback);

  /**
   * @brief Cancel a subscription.
   * @param[in] token Token returned by subscribeToDeviceGroup or subscribeToStateChanges.
   */
  void unsubscribe(subscription::SubscriberToken token);

  /**
   * @brief Return the current health state of the state machine worker.
   * @return Current manager state.
   */
  ManagerState getManagerState() const noexcept;

  /**
   * @brief Return the parameters used to initialize the manager.
   * @return Initial parameter struct.
   */
  ManagerParams getParams() const noexcept;

  /**
   * @brief Return the SensorRing managed by this manager.
   * @return Pointer to the managed SensorRing (never null while manager is alive).
   */
  ring::SensorRing* getSensorRing() const noexcept;

  /**
   * @brief Queue a callable to run once in the next extra-actions slot of the state machine.
   * @param[in] action Callable executed once from the measurement thread; should be non-blocking and exception-safe.
   */
  void enqueueExtraAction(std::function<void()> action);

private:
  std::unique_ptr<MeasurementManagerImpl> _mm_impl;
};

} // namespace manager

} // namespace eduart