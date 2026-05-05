// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ManagerParams.hpp
 * @author EduArt Robotik GmbH
 * @brief  Parameter structure for the MeasurementManager.
 * @date   2026-02-19
 */

#pragma once

#include <chrono>

namespace eduart {

namespace sensorring {

namespace manager {

/**
 * @struct ManagerParams
 * @brief Parameter structure of the MeasurementManager. The MeasurementManager
 * handles the timing and communication of the whole system by running the
 * measurement state machine. One measurement manager manages exactly one sensor ring.
 */
struct ManagerParams {
  /// Timeout for individual sensor operations before the error handler is called.
  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);

  /// Enable bit rate switching on the CAN bus interface.
  bool enable_brs = false;

  /// If set to true error handling is enabled to try to repair communication and timing errors.
  /// When set to false the MeasurementManager instantly shuts down when an error is detected.
  bool repair_errors = true;

  /// Target frequency cap for depth sensors (Hz, tenths precision).
  /// 0.0 = run each group at its hardware max rate.
  double frequency_tof_hz = 0.0;

  /// Target frequency cap for thermal sensors (Hz, tenths precision).
  /// 0.0 = run each group at its hardware max rate.
  double frequency_thermal_hz = 6.0;
};

} // namespace manager

} // namespace sensorring

} // namespace eduart