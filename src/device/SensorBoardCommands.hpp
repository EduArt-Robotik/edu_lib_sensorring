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

namespace device {

/**
 * @brief Reset all boards on all interfaces (broadcast reset command).
 * @return true on success.
 */
bool resetBoards();

/**
 * @brief Send bit-rate switching command on the given interface.
 * @param[in] interface Communication interface ID.
 * @param[in] enable Whether to enable BRS.
 */
void cmdSetBitRateSwitching(com::ComInterfaceID interface, bool enable);

/**
 * @brief Send enumeration command on the given interface so boards respond with CMD_ACTIVE_DEVICE_RESPONSE.
 * @param[in] interface Communication interface ID to enumerate.
 */
void cmdEnumerateBoards(com::ComInterfaceID interface);

} // namespace device

} // namespace sensorring

} // namespace eduart
