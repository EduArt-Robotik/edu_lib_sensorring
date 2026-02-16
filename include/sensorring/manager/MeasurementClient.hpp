// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementClient.hpp
 * @author EduArt Robotik GmbH
 * @brief  MeasurementClient that can be registered with the MeasurementManager to receive measurement data
 * @date   2024-12-25
 */

#pragma once

#include <unordered_map>
#include <unordered_set>

#include "sensorring/device/DeviceGroup.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/SubscriberToken.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"
#include "sensorring/types/TofMeasurement.hpp"

namespace eduart {

namespace manager {

/**
 * @class MeasurementClient
 * @brief Observer interface of the MeasurementManager class. Defines the
 * callback methods that are triggered by the MeasurementManager. It is possible
 * to implement only one or a selection of the callback methods.
 */
class SENSORRING_EXPORT MeasurementClient {
public:
  /// Destructor
  virtual ~MeasurementClient();

  /**
   * Register the client to the MeasurementManager
   * @param[in] manager the MeasurementManager to register to
   */
  bool registerClient(MeasurementManager* manager);

  /**
   * Unregister the client from the MeasurementManager
   */
  bool unregisterClient();

  /**
   * Unregister the client from a specific MeasurementManager
   * @param[in] manager the MeasurementManager to unregister from
   */
  bool unregisterClient(MeasurementManager* manager);

  /**
   * Callback method for state changes of the state machine worker
   * @param[in] state the new state of the state machine worker
   */
  virtual void onStateChange([[maybe_unused]] const ManagerState state) {};

  /**
   * Callback method for new Time-of-Flight sensor measurements. Returns a
   * vector of the raw measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the individual sensor coordinate frames
   */
  virtual void onRawTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) {};

  /**
   * Callback method for new Time-of-Flight sensor measurements. Returns a
   * vector of the transformed measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the common transformed coordinate frame
   */
  virtual void onTransformedTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) {};

  /**
   * Callback method for new thermal sensor measurements. Returns a
   * vector of the measurements from all sensors.
   * @param[in] measurement the most recent thermal sensor
   * measurements in the common transformed coordinate frame
   */
  virtual void onThermalMeasurement([[maybe_unused]] const std::vector<measurement::ThermalMeasurement>& measurement_vec) {};

private:
  std::unordered_set<MeasurementManager*> _managers;
  std::unordered_map<MeasurementManager*, SubscriberToken> _state_subscriptions;
  std::unordered_map<MeasurementManager*, SubscriberToken> _tof_subscriptions;
  std::unordered_map<MeasurementManager*, SubscriberToken> _thermal_subscriptions;

  void onTofDispatcher(const device::DeviceGroup& group);
  void onThermalDispatcher(const device::DeviceGroup& group);
};

} // namespace manager

} // namespace eduart