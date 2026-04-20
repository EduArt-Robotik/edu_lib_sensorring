// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorRing.hpp
 * @author EduArt Robotik GmbH
 * @brief  Top-level container managing multiple sensor buses
 * @date   2025-02-06
 */

#pragma once

#include <memory>
#include <vector>

#include "sensorring/platform/SensorringExport.hpp"

#include "SensorBus.hpp"

namespace eduart {

namespace sensorring {

namespace ring {

/**
 * @class SensorRing
 * @brief Top-level container managing multiple sensor buses and coordinating operations across them.
 */
class SENSORRING_EXPORT SensorRing {

public:
  /**
   * @brief Constructor
   * @param[in] bus_vec Vector of sensor buses to manage
   */
  SensorRing(std::vector<std::unique_ptr<SensorBus> > bus_vec);

  /// Destructor
  ~SensorRing();

  /**
   * @brief Get all sensor bus interfaces managed by this ring.
   * @return Vector of const pointers to all sensor buses.
   */
  std::vector<SensorBus*> getSensorBuses() const;

  /**
   * @brief Enable or disable bit rate switching on CAN bus interfaces.
   * @param[in] brs_enable Enable flag
   */
  void setBitRateSwitching(bool brs_enable);

  /**
   * @brief Get all devices connected to the sensor ring.
   * @return Vector of all devices.
   */
  std::vector<device::IDevice*> getDevices() const;

private:
  /// Vector of sensor buses managed by this sensor ring.
  std::vector<std::unique_ptr<SensorBus> > _bus_vec;
};

} // namespace ring

} // namespace sensorring

} // namespace eduart