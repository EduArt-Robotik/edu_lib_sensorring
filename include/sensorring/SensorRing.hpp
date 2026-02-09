// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   SensorRing.hpp
 * @author EduArt Robotik GmbH
 * @brief  Top-level container managing multiple sensor buses
 * @date   2025-02-06
 */

#pragma once

#include <memory>
#include <vector>

#include "device/BaseDevice.hpp"
#include "sensorring/Parameter.hpp"

#include "SensorBus.hpp"

namespace eduart {

namespace ring {

/**
 * @class SensorRing
 * @brief Top-level container managing multiple sensor buses and coordinating operations across them.
 */
class SensorRing {

public:
  /**
   * Constructor
   * @param[in] params Configuration parameters for the sensor ring
   * @param[in] bus_vec Vector of sensor buses to manage
   */
  SensorRing(RingParams params, std::vector<std::unique_ptr<bus::SensorBus> > bus_vec);
  /// Destructor
  ~SensorRing();

  /**
   * @brief Get all sensor bus interfaces managed by this ring.
   * @return Vector of const pointers to all sensor buses.
   */
  std::vector<const bus::SensorBus*> getInterfaces() const;

  /**
   * @brief Enable or disable bit rate switching on CAN bus interfaces.
   * @param[in] brs_enable Enable flag
   */
  void setBrs(bool brs_enable);

  /**
   * @brief Get all devices connected to the sensor ring.
   * @return Vector of all devices.
   */
  std::vector<device::BaseDevice*> getDevices() const;

  /**
   * @brief Reset all devices on all sensor buses.
   */
  void resetDevices();

  /**
   * @brief Reset sensor state on all sensor buses.
   */
  void resetSensorState();

  /**
   * @brief Enumerate devices on all sensor buses.
   * @return true if enumeration succeeded on all buses
   */
  bool enumerateDevices();

  /**
   * @brief Create a SensorRing instance from configuration parameters.
   * @param[in] params Configuration parameters for the sensor ring
   * @return Unique pointer to the created SensorRing instance
   */
  static std::unique_ptr<SensorRing> create(RingParams params);

private:
  const RingParams _params;
  std::vector<std::unique_ptr<bus::SensorBus> > _bus_vec;
};

} // namespace ring

} // namespace eduart