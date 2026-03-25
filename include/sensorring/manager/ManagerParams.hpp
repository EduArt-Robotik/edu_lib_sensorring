// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ManagerParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the MeasurementManager.
 * @date   2026-02-19
 */

#pragma once

#include <chrono>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

/**
 * @struct ManagerParams
 * @brief Parameter structure of the MeasurementManager. The MeasurementManager
 * handles the timing and communication of the whole system by running the
 * measurement state machine. One measurement manager manages exactly one sensor ring.
 */
struct SENSORRING_EXPORT ManagerParams {
  /// Timeout for the measurements before the error handler is called.
  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);

  /// Enable bit rate switching on the can bus interface.
  bool enable_brs = false; // ToDo: remove

  /// If set to true error handling is enabled to try to repair communication and timing errors. When set to false the MeasurementManager instantly shuts down when an error is detected.
  bool repair_errors = true;

  /// Target frequency for the time of flight measurement. If set to 0.0 the measurements are executed as fast as possible.
  double frequency_tof_hz = 0.0;

  /// Target frequency for the thermal measurement. If set to 0.0 the measurements are executed as fast as possible.
  double frequency_thermal_hz = 5.0;
};

} // namespace manager

} // namespace eduart