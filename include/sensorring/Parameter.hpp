// Copyright (c) 2025 EduArt Robotik GmbH

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

#include "sensorring/math/Math.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/InterfaceType.hpp"

namespace eduart {

namespace sensor {

/**
 * @enum SensorOrientation
 * @brief Possible orientations of a sensor board. Used to rotate the thermal images and to mirror the light animations.
 */
enum class SENSORRING_API Orientation {
  left,
  right,
  none
};

/**
 * @struct LightParams
 * @brief Parameter structure of the sensor lights of a sensor board. Not all sensor boards have lights.
 */
struct SENSORRING_API LightParams {
  /// Enable the lights.
  bool enable = false;

  /// Orientation of the sensor board. Used to to mirror the light animations.
  Orientation orientation = Orientation::none;
};

/**
 * @struct ThermalSensorParams
 * @brief Parameter structure of the thermal sensor of a sensor board. Not all sensor boards have thermal sensors.
 */
struct SENSORRING_API ThermalSensorParams {
  /// Customizable index that is returned with every measurement from this sensor.
  int user_idx = 0;

  /// Enable thermal measurements from this sensor.
  bool enable = false;

  /// Minimal temperature in degree celsius used for color mapping of the thermal images. Only used when auto_min_max is set to false.
  double t_min_deg_c = 0;

  /// Maximal temperature in degree celsius used for color mapping of the thermal images. Only used when auto_min_max is set to false.
  double t_max_deg_c = 0;

  /// Enable automatic color scaling of the thermal images using the coldest and the hottest temperature in each image.
  bool auto_min_max = false;

  /// Save the thermal sensors eeprom content to a local file to only require a transfer once.
  bool use_eeprom_file = false;

  /// Save the calibration data for the thermal sensor to a local file to only require the calibration procedure once.
  bool use_calibration_file = false;

  /// Directory of the eeprom file. The user requires read and write access to this directory.
  std::string eeprom_dir = "";

  /// Directory of the calibration data file.  The user requires read and write access to this directory.
  std::string calibration_dir = "";

  /// Orientation of the sensor board. Used to flip the image upside down.
  Orientation orientation = Orientation::none;
};

/**
 * @struct TofSensorParams
 * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
 */
struct SENSORRING_API TofSensorParams {
  /// Customizable index that is returned with every measurement from this sensor.
  int user_idx = 0;

  /// Enable time of flight measurements from this sensor.
  bool enable = false;
};

/**
 * @struct SensorBoardParams
 * @brief Parameter structure of a sensor board. A sensor board is one circuit board.
 */
struct SENSORRING_API SensorBoardParams {
  /// Rotation part of the sensors pose. The rotation is applied in the order Roll(x) - Pitch(y) - Yaw(z). Values: Euler angles in degrees
  math::Vector3 rotation = { 0, 0, 0 };

  /// Translation part of the sensor pose. Values: XYZ coordinates in meters.
  math::Vector3 translation = { 0, 0, 0 };

  /// Parameters of the lights on the sensor board. Only applicable if the corresponding hardware actually has addressable lights.
  LightParams light_params;

  /// Parameters of the time of flight sensor on the sensor board. Only applicable if the corresponding hardware actually has a time of flight sensor.
  TofSensorParams tof_params;

  /// Parameters of the thermal sensor on the sensor board. Only applicable if the corresponding hardware actually has a thermal sensor.
  ThermalSensorParams thermal_params;
};

} // namespace sensor

namespace bus {

/**
 * @struct BusParams
 * @brief Parameter structure of a communication bus. A bus is one communication
 * interface e.g. CAN bus and has an arbitrary number of sensor boards connected.
 */
struct SENSORRING_API BusParams {
  /// Name of the communication interface. E.g. "can0" for a CAN bus.
  std::string interface_name;

  /// Type of the communication interface. Only interface types that are defined in com::InterfaceType are currently supported.
  com::InterfaceType type = com::InterfaceType::UNDEFINED;

  /// Parameters of the sensor boards that are connected through this communication interface. Each element belongs to a unique sensor board.
  std::vector<sensor::SensorBoardParams> board_param_vec;
};

} // namespace bus

namespace ring {

/**
 * @struct RingParams
 * @brief Parameter structure of a sensor ring. The sensor ring is the
 * abstraction of the whole sensor system and consists of an arbitrary number of
 * communication interfaces.
 */
struct SENSORRING_API RingParams {
  /// Timeout for the measurements before the error handler is called.
  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);

  /// Parameters of the communication interfaces that will be included in the sensor ring. Each element belongs to a unique communication interface.
  std::vector<bus::BusParams> bus_param_vec;
};

} // namespace ring

namespace manager {

/**
 * @struct ManagerParams
 * @brief Parameter structure of the MeasurementManager. The MeasurementManager
 * handles the timing and communication of the whole system by running the
 * measurement state machine. One measurement manager manages exactly one sensor ring.
 */
struct SENSORRING_API ManagerParams {
  /// Enable bit rate switching on the can bus interface.
  bool enable_brs = false;

  /// If set to true a formatted string describing the sensor topology is printed via the Logger when the MeasurementManager init() method is called.
  bool print_topology = true;

  /// If set to true error handling is enabled to try to repair communication and timing errors. When set to false the MeasurementManager instantly shuts down when an error is detected.
  bool repair_errors = true;

  /// If set to true the MeasurementManager will only start when the configured topology matches the actual connected devices. If set to false the MeasurementManager will still start but only use the properly configured sensors.
  bool enforce_topology = false;

  /// Target frequency for the time of flight measurement. If set to 0.0 the measurements are executed as fast as possible.
  double frequency_tof_hz = 0.0;

  /// Target frequency for the thermal measurement. If set to 0.0 the measurements are executed as fast as possible.
  double frequency_thermal_hz = 1.0;

  /// Parameters of the sensor ring that will be managed by the MeasurementManager.
  ring::RingParams ring_params;
};

} // namespace manager

} // namespace eduart