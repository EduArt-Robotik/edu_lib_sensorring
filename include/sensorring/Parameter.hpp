// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Parameter.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure used to initialize the sensorring
 * @date   2024-11-22
 */

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "sensorring/device/hardware/SensorBoardType.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace bus {

/**
 * @struct BusTopology
 * @brief Topology of a communication bus.
 */
struct SENSORRING_EXPORT BusTopology {
  /// Communication interface parameters.
  com::ComInterfaceID interface;

  /// Types of the sensor boards on the bus.
  std::vector<device::SensorBoardType> board_type_vec;
};

} // namespace bus

namespace ring {

/**
 * @struct RingTopology
 * @brief Topology of the sensor ring.
 */
struct SENSORRING_EXPORT RingTopology {
  /// Topology of the communication buses.
  std::vector<bus::BusTopology> bus_topology_vec;
};

} // namespace ring

} // namespace eduart