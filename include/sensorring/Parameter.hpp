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
#include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Params.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/InterfaceType.hpp"

namespace eduart {

namespace device {
/**
 * @struct SensorBoardParams
 * @brief Parameter structure of a sensor board. A sensor board is one circuit board.
 */
struct SENSORRING_EXPORT SensorBoardParams {
  /// Hardware board type. When set to Undefined, the board is created with all supported device types (backward compatibility).
  SensorBoardType board_type = SensorBoardType::Undefined;

  /// Rotation part of the sensors pose. The rotation is applied in the order Roll(x) - Pitch(y) - Yaw(z). Values: Euler angles in degrees
  math::Vector3 rotation = { 0, 0, 0 };

  /// Translation part of the sensor pose. Values: XYZ coordinates in meters.
  math::Vector3 translation = { 0, 0, 0 };

  /// Parameters of the lights on the sensor board. Only applicable if the corresponding hardware actually has addressable lights.
  WS2812b_Params ws2812b_params;

  /// Parameters of the time of flight sensor on the sensor board. Only applicable if the corresponding hardware actually has a time of flight sensor.
  VL53L8CX_Params vl53l8cx_params;

  /// Parameters of the thermal sensor on the sensor board. Only applicable if the corresponding hardware actually has a thermal sensor.
  HTPA32_Params htpa32_params;
};

} // namespace device

namespace bus {

/**
 * @struct BusParams
 * @brief Parameter structure of a communication bus. A bus is one communication
 * interface e.g. CAN bus and has an arbitrary number of sensor boards connected.
 */
struct SENSORRING_EXPORT BusParams {
  /// Name of the communication interface. E.g. "can0" for a CAN bus.
  std::string interface_name;

  /// Type of the communication interface. Only interface types that are defined in com::InterfaceType are currently supported.
  com::InterfaceType type = com::InterfaceType::UNDEFINED;

  /// Parameters of the sensor boards that are connected through this communication interface. Each element belongs to a unique sensor board.
  std::vector<device::SensorBoardParams> board_param_vec;
};

} // namespace bus

namespace ring {

/**
 * @enum RingCreationMode
 * @brief How the sensor ring is created: from explicit configuration or by auto-detecting connected hardware.
 */
enum class RingCreationMode {
  /// Use bus_param_vec and board_param_vec to build the ring; optionally enforce that physical hardware matches (via ManagerParams::enforce_topology).
  Configured,
  /// Enumerate each bus and create one SensorBoard per discovered board; board_param_vec is ignored; default_board_params is applied to every discovered board.
  AutoDetect
};

/**
 * @struct RingParams
 * @brief Parameter structure of a sensor ring. The sensor ring is the
 * abstraction of the whole sensor system and consists of an arbitrary number of
 * communication interfaces.
 */
struct SENSORRING_EXPORT RingParams {
  /// How to create the ring: Configured (use board_param_vec) or AutoDetect (enumerate and create from discovered boards).
  RingCreationMode creation_mode = RingCreationMode::Configured;

  /// Default board parameters used when creation_mode is AutoDetect; applied to every discovered board. Ignored when creation_mode is Configured.
  device::SensorBoardParams default_board_params;

  /// Parameters of the communication interfaces. For Configured: each bus lists its boards in board_param_vec. For AutoDetect: only interface_name and type are used; boards are discovered.
  std::vector<bus::BusParams> bus_param_vec;
};

} // namespace ring

} // namespace eduart