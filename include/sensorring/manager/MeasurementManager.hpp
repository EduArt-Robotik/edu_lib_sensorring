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
#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/device/DepthSensor.hpp"
#include "sensorring/device/Group.hpp"
#include "sensorring/device/Light.hpp"
#include "sensorring/device/ThermalSensor.hpp"
#include "sensorring/manager/ManagerParams.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

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
   * @brief Construct the manager from a configured factory. Builds the SensorRing internally.
   * @param[in] params Manager configuration.
   * @param[in] factory Configured SensorRingFactory (interfaces and expectations already set).
   */
  MeasurementManager(ManagerParams params, ring::SensorRingFactory& factory);

  /**
   * @brief Construct the manager from a pre-built SensorRing (expert-user constructor).
   * @param[in] params Manager configuration.
   * @param[in] sensor_ring Fully configured SensorRing. Ownership is transferred.
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
  subscription::Subscription subscribeToStateChanges(std::function<void(const ManagerState state)> callback);

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
   * @brief Return a typed group of all devices matching the given interface.
   * @tparam T Device interface type (DepthSensor, ThermalSensor, Light).
   * @return Group<T> wrapping matching device pointers.
   */
  template <typename T> device::Group<T> devices() const noexcept;

  /// Convenience: return all depth sensors.
  device::Group<device::DepthSensor> depthSensors() const noexcept;

  /// Convenience: return all thermal sensors.
  device::Group<device::ThermalSensor> thermalSensors() const noexcept;

  /// Convenience: return all lights.
  device::Group<device::Light> lights() const noexcept;

private:
  std::unique_ptr<MeasurementManagerImpl> _mm_impl;
};

} // namespace manager

} // namespace sensorring

} // namespace eduart