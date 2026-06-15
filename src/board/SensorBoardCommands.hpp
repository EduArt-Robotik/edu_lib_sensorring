// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoardCommands.hpp
 * @author EduArt Robotik GmbH
 * @brief  Internal low-level CAN commands for sensor boards (reset, enumeration, BRS).
 * @date   2026-03-27
 */

#pragma once

#include "sensorring/interface/ComInterfaceID.hpp"

namespace eduart {

namespace sensorring {

namespace board {

/**
 * @brief Reset all boards on all interfaces (broadcast reset command).
 * @return true on success.
 */
bool resetBoards();

/**
 * @brief Send enumeration command on the given interface so boards respond with CMD_ACTIVE_DEVICE_RESPONSE.
 * @param[in] interface Communication interface ID to enumerate.
 */
void cmdEnumerateBoards(com::ComInterfaceID interface);

} // namespace board

} // namespace sensorring

} // namespace eduart
