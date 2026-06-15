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

#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/board/SensorBoardType.hpp"
#include "sensorring/device/Device.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace board {

using device::Device;
using device::DeviceID;
using device::DevicePoseOffset;
using device::DeviceType;

struct SensorBoardParams;
class SensorBoard;

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
class SENSORRING_EXPORT SensorBoardManager {
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

  /// Map of device parameters indexed by device type.
  using DeviceParamsVariant = std::variant<device::VL53L8CX_Params, device::HTPA32_Params, device::WS2812b_Params, device::TMF8829_Params>;

  /// Container for device parameters keyed by device type.
  using DeviceParamsMap = std::unordered_map<DeviceType, DeviceParamsVariant>;

  /**
   * @brief Create a basic SensorBoard from enumeration information.
   * @param[in] enum_info Enumeration result for this board.
   * @param[in] params Board configuration parameters.
   * @param[in] interface Communication interface ID.
   * @param[in] idx Board index on the bus.
   * @return Unique pointer to the created SensorBoard.
   */
  static std::unique_ptr<SensorBoard> createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx);

  /**
   * @brief Create a SensorBoard from enumeration info, instantiating only the device types present in @p device_params_map and applying the provided params.
   * @param[in] enum_info Enumeration result for this board.
   * @param[in] params Board configuration parameters.
   * @param[in] interface Communication interface ID.
   * @param[in] idx Board index on the bus.
   * @param[in] device_params_map Map from DeviceType to params; only these device types are created.
   * @return Unique pointer to the created SensorBoard.
   */
  static std::unique_ptr<SensorBoard> createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx, const DeviceParamsMap& device_params_map);

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
            { DeviceType::TMF8829, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } },
        } } },
    { SensorBoardType::Undefined,
     { "Unknown",
        {
            // No devices defined
        } } }
  };
};

} // namespace board

} // namespace sensorring

} // namespace eduart