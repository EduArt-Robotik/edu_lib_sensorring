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
#include "sensorring/device/hardware/SensorBoardType.hpp"

namespace eduart {

namespace com {
class ComInterface;
} // namespace com

namespace device {

struct SensorBoardParams;
struct SensorBoard;

/**
 * @struct VL53L8CX_DeviceInfo
 * @brief Static info for a VL53L8CX ToF device (name, FOV, resolution, max rate).
 */
struct VL53L8CX_DeviceInfo {
  /// Device name.
  std::string_view name;
  /// Field of view x in degrees.
  double fov_x;
  /// Field of view y in degrees.
  double fov_y;
  /// Resolution in x.
  int res_x;
  /// Resolution in y.
  int res_y;
  /// Maximum sample rate in Hz.
  double max_rate;
};

/**
 * @struct HTPA32_DeviceInfo
 * @brief Static info for an HTPA32 thermal device (name, resolution, max rate).
 */
struct HTPA32_DeviceInfo {
  /// Device name.
  std::string_view name;
  /// Resolution in x.
  int res_x;
  /// Resolution in y.
  int res_y;
  /// Maximum sample rate in Hz.
  double max_rate;
};

/**
 * @struct WS2812b_DeviceInfo
 * @brief Static info for a WS2812b LED device (name, LED count).
 */
struct WS2812b_DeviceInfo {
  /// Device name.
  std::string_view name;
  /// Number of LEDs.
  unsigned int count;
};

/// Variant of device-specific static info (VL53L8CX, HTPA32, WS2812b) or empty.
using AnyDeviceInfo = std::variant<std::monostate, VL53L8CX_DeviceInfo, HTPA32_DeviceInfo, WS2812b_DeviceInfo>;

/**
 * @struct BoardDeviceInfo
 * @brief Description of one physical device on a board: ID, pose offset, and type-specific info.
 */
struct BoardDeviceInfo {
  /// Logical device ID (type, name, index).
  DeviceID id;
  /// Pose offset relative to board center.
  DevicePoseOffset pose_offset;
  /// Type-specific static info (VL53L8CX, HTPA32, or WS2812b).
  AnyDeviceInfo info;
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
      if (dev.id.getType() == id.getType()) {
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
   * @brief Look up type-specific static device info for a device on a given board type.
   * @param[in] board_type Board type.
   * @param[in] id Device ID (type used for lookup).
   * @return Pointer to AnyDeviceInfo if found, nullptr otherwise.
   */
  static inline const AnyDeviceInfo* getDeviceInfo(SensorBoardType board_type, const DeviceID& id) {
    const auto& board = sensorBoardDatabase.at(board_type);
    for (const auto& dev : board.devices) {
      if (dev.id.getType() == id.getType()) {
        return &dev.info;
      }
    }
    return nullptr;
  }

  /**
   * @brief Create a SensorBoard for the given board type using the static board database; device set depends on board type (or all types if Undefined).
   * @param[in] board_type Hardware board type (Headlight, Taillight, etc.; Undefined creates all supported device types).
   * @param[in] params Board configuration parameters.
   * @param[in] interface Communication interface for the board.
   * @param[in] idx Board index on the bus.
   * @return Unique pointer to the created SensorBoard.
   */
  static std::unique_ptr<SensorBoard> createSensorBoard(SensorBoardType board_type, const SensorBoardParams& params, com::ComInterface* interface, unsigned int idx);

private:
  // Per-device-type static infos (shared across all boards).
  static inline const VL53L8CX_DeviceInfo VL53L8CX_INFO{ "ST VL53L8CX", 45.0, 45.0, 8, 8, 15.0 };

  static inline const HTPA32_DeviceInfo HTPA32_INFO{ "Heimann HTPA32", 32, 32, 15.0 };

  static inline const std::unordered_map<SensorBoardType, SensorBoardInfo> sensorBoardDatabase = {
    { SensorBoardType::Headlight,
     { "Headlight",
        {
            BoardDeviceInfo{ DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, VL53L8CX_INFO },
            BoardDeviceInfo{ DeviceID{ DeviceType::HTPA32, "Thermal Sensor", 0 }, DevicePoseOffset{ { 0.013, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, HTPA32_INFO },
            BoardDeviceInfo{ DeviceID{ DeviceType::WS2812b, "Light", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, WS2812b_DeviceInfo{ "WS2812b (11 LEDs)", 11 } },
        } } },
    { SensorBoardType::Taillight,
     { "Taillight",
        {
            BoardDeviceInfo{ DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, VL53L8CX_INFO },
            BoardDeviceInfo{ DeviceID{ DeviceType::WS2812b, "Light", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, WS2812b_DeviceInfo{ "WS2812b (8 LEDs)", 8 } },
        } } },
    { SensorBoardType::Sidepanel,
     { "Sidepanel",
        {
            BoardDeviceInfo{ DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, VL53L8CX_INFO },
            BoardDeviceInfo{ DeviceID{ DeviceType::WS2812b, "Light", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, WS2812b_DeviceInfo{ "WS2812b (2 LEDs)", 2 } },
        } } },
    { SensorBoardType::Minipanel,
     { "Minipanel",
        {
            BoardDeviceInfo{ DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 }, DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } }, VL53L8CX_INFO },
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