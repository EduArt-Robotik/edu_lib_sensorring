// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoardManager.hpp
 * @author EduArt Robotik GmbH
 * @brief  Static board database and factory for creating SensorBoards by hardware type.
 * @date   2025-02-19
 */

#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/EnumerationInformation.hpp"
#include "sensorring/device/hardware/SensorBoardType.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"

namespace eduart {

namespace device {

struct SensorBoardParams;
struct SensorBoard;

/**
 * @struct BoardDeviceInfo
 * @brief Description of one physical device on a board: ID, pose offset, and type-specific info.
 */
struct BoardDeviceInfo {
  /// Logical device ID (type, name, index).
  DeviceType type;
  /// Pose offset relative to board center.
  DevicePoseOffset pose_offset;
};

/**
 * @struct SensorBoardInfo
 * @brief Description of a sensor board hardware variant: name and list of devices.
 */
struct SensorBoardInfo {
  /// Board variant name (e.g. "Headlight", "Taillight").
  std::string_view name;
  /// Devices on this board type.
  std::vector<BoardDeviceInfo> devices;
};

/**
 * @class SensorBoardManager
 * @brief Provides the static board database and factory to create SensorBoard instances by board type.
 */
class SensorBoardManager {
public:
  /**
   * @brief Look up static board info for a given board type.
   * @param[in] type Board type (Headlight, Taillight, etc.).
   * @return Const reference to SensorBoardInfo for that type.
   */
  static inline const SensorBoardInfo& getSensorBoardInfo(SensorBoardType type) { return sensorBoardDatabase.at(type); }

  /**
   * @brief Pose offset of a device relative to the board center for the given board type and device ID.
   * @param[in] board_type Board type.
   * @param[in] id Device ID (type used for lookup).
   * @return DevicePoseOffset; zero if device not found in board config.
   */
  static inline DevicePoseOffset getDevicePoseOffset(SensorBoardType board_type, const DeviceID& id) {
    const auto& board = sensorBoardDatabase.at(board_type);
    for (const auto& dev : board.devices) {
      if (dev.type == id.getType()) {
        return dev.pose_offset;
      }
    }
    // Fallback: zero offset if not found.
    return DevicePoseOffset{
      { 0.0, 0.0, 0.0 },
      { 0.0, 0.0, 0.0 }
    };
  }

  /**
   * @brief Create a SensorBoard for the given board type using the static board database; device set depends on board type (or all types if Undefined).
   * @param[in] board_type Hardware board type (Headlight, Taillight, etc.; Undefined creates all supported device types).
   * @param[in] params Board configuration parameters.
   * @param[in] interface Communication interface ID for the board.
   * @param[in] idx Board index on the bus.
   * @return Unique pointer to the created SensorBoard.
   */
  static std::unique_ptr<SensorBoard> createSensorBoard(SensorBoardType board_type, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx); //ToDo: Maybe remove this overload

  static std::unique_ptr<SensorBoard> createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx);

private:
  static inline const std::unordered_map<SensorBoardType, SensorBoardInfo> sensorBoardDatabase = {
    { SensorBoardType::Headlight,
     { "Headlight",
        {
            { DeviceType::VL53L8CX, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
            { DeviceType::HTPA32, DevicePoseOffset{ { 0.013, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
            { DeviceType::WS2812b, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
        } } },
    { SensorBoardType::Taillight,
     { "Taillight",
        {
            { DeviceType::VL53L8CX, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
            { DeviceType::WS2812b, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
        } } },
    { SensorBoardType::Sidepanel,
     { "Sidepanel",
        {
            { DeviceType::VL53L8CX, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
            { DeviceType::WS2812b, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
        } } },
    { SensorBoardType::Minipanel,
     { "Minipanel",
        {
            { DeviceType::VL53L8CX, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
        } } },
    { SensorBoardType::Undefined,
     { "Unknown",
        {
            // No devices defined
        } } }
  };
};

} // namespace device

} // namespace eduart