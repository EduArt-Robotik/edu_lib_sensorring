// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorRing.hpp
 * @author EduArt Robotik GmbH
 * @brief  Top-level container managing multiple sensor buses
 * @date   2025-02-06
 */

#pragma once

#include <memory>
#include <string>
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
   * @brief Constructor
   * @param[in] bus_vec Vector of sensor buses to manage
   */
  SensorRing(std::vector<std::unique_ptr<bus::SensorBus> > bus_vec);

  /// Destructor
  ~SensorRing();

  /**
   * @brief Get all sensor bus interfaces managed by this ring.
   * @return Vector of const pointers to all sensor buses.
   */
  std::vector<bus::SensorBus*> getSensorBuses() const;

  /**
   * @brief Enable or disable bit rate switching on CAN bus interfaces.
   * @param[in] brs_enable Enable flag
   */
  void setBrs(bool brs_enable);

  /**
   * @brief Get all devices connected to the sensor ring.
   * @return Vector of all devices.
   */
  std::vector<device::IDevice*> getDevices() const;

  /**
   * @brief Get a string representation of the topology of the connected sensors.
   * @return Formatted string describing the topology
   */
  std::string printTopology() noexcept;

  /**
   * @brief Get the topology of the sensor ring.
   * @return RingTopology
   */
  RingTopology getTopology() const noexcept;

  /**
   * @brief Verify if the topology of the sensor ring matches the actual connected hardware.
   * @return true if the topology matches, false otherwise
   */
  bool verifyTopology() const;

  /**
   * @brief Enumerate the connected devices that are connected on the specified interfaces and create a sensor ring from what is connected.
   * @param[in] interfaces Vector of communication interface parameters.
   * @return Unique pointer to the created SensorRing instance. Returns nullptr if no sensor boards were found on any of the provided interfaces.
   */
  static std::unique_ptr<SensorRing> createFromEnumeration(std::vector<com::ComInterfaceID> interfaces);

private:
  /// Topology of the sensor ring as configured upon creation.
  RingTopology _topology;

  /// Vector of sensor buses managed by this sensor ring.
  std::vector<std::unique_ptr<bus::SensorBus> > _bus_vec;
};

} // namespace ring

} // namespace eduart