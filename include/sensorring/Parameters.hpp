#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "CustomTypes.hpp"

using namespace std::chrono_literals;

namespace eduart {

namespace sensor {

/**
 * @enum SensorOrientation
 * @brief Possible orientations of a sensor board. Used to rotate the thermal
 * images.
 */
enum class Orientation {
  left,
  right,
  none
};

/**
 * @enum LightParams
 * @brief Parameter structure of the sensor lights of a sensor board.
 */
struct LightParams {
  bool enable             = false;
  Orientation orientation = Orientation::none;
};

/**
 * @enum ThermalSensorParams
 * @brief Parameter structure of the thermal sensor of a sensor board. Not all
 * sensor boards have thermal sensors.
 */
struct ThermalSensorParams {
  int user_idx                = 0;
  bool enable                 = false;
  double t_min_deg_c          = 0;
  double t_max_deg_c          = 0;
  bool auto_min_max           = false;
  bool use_eeprom_file        = false;
  bool use_calibration_file   = false;
  std::string eeprom_dir      = "";
  std::string calibration_dir = "";
  math::Vector3 rotation      = { 0, 0, 0 };
  math::Vector3 translation   = { 0, 0, 0 };
  Orientation orientation     = Orientation::none;
};

/**
 * @enum TofSensorParams
 * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
 */
struct TofSensorParams {
  int user_idx              = 0;
  bool enable               = false;
  math::Vector3 rotation    = { 0, 0, 0 };
  math::Vector3 translation = { 0, 0, 0 };
};

/**
 * @enum SensorBoardParams
 * @brief Parameter structure of a sensor board. A sensor board is one circuit
 * board.
 */
struct SensorBoardParams {
  math::Vector3 rotation    = { 0, 0, 0 };
  math::Vector3 translation = { 0, 0, 0 };
  LightParams led_params;
  TofSensorParams tof_params;
  ThermalSensorParams thermal_params;
};

} // namespace sensor

namespace bus {

/**
 * @enum BusParams
 * @brief Parameter structure of a communication bus. A bus is one communication
 * interface e.g. can bus and has an arbitrary number of sensor boards
 * connected.
 */
struct BusParams {
  std::string interface_name;
  com::InterfaceType type = com::InterfaceType::UNDEFINED;
  std::vector<sensor::SensorBoardParams> board_param_vec;
};

} // namespace bus

namespace ring {

/**
 * @enum RingParams
 * @brief Parameter structure of a sensor ring. The sensor ring is the
 * abstraction of the whole sensor system and consists of an arbitrary number of
 * communication interfaces.
 */
struct RingParams {
  std::chrono::milliseconds timeout = 1000ms;
  std::vector<bus::BusParams> bus_param_vec;
};

} // namespace ring

namespace manager {

/**
 * @enum ManagerParams
 * @brief Parameter structure of the MeasurementManager. The MeasurementManager
 * handles the timing and communication of the whole system by running a looping
 * state machine. One measurement manager manages exactly one sensor ring.
 */
struct ManagerParams {
  bool enable_brs             = false;
  bool print_topology         = false;
  double frequency_tof_hz     = 0.0;
  double frequency_thermal_hz = 1.0;
  ring::RingParams ring_params;
};

} // namespace manager

} // namespace eduart