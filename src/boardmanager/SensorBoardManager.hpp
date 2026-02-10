#pragma once

#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"

namespace eduart {

namespace device {

// Numbers match the definition in the sensor board firmware
enum class SensorBoardType {
  Sidepanel = 0x00,
  Headlight = 0x01,
  Taillight = 0x02,
  Minipanel = 0x03,
  Undefined = 0xff
};

// Type-specific static information for different concrete devices.
struct VL53L8CX_DeviceInfo {
  std::string_view name;
  double fov_x;
  double fov_y;
  int res_x;
  int res_y;
  double max_rate;
};

struct HTPA32_DeviceInfo {
  std::string_view name;
  int res_x;
  int res_y;
  double max_rate;
};

struct WS2812b_DeviceInfo {
  std::string_view name;
  unsigned int count;
};

using AnyDeviceInfo = std::variant<std::monostate, VL53L8CX_DeviceInfo, HTPA32_DeviceInfo, WS2812b_DeviceInfo>;

// Description of a single physical device on a board.
struct BoardDeviceInfo {
  DeviceID         id;          // logical device id (type, name, index)
  DevicePoseOffset pose_offset; // pose offset relative to board center
  AnyDeviceInfo    info;        // type-specific static info
};

// Description of a sensor board hardware variant.
struct SensorBoardInfo {
  std::string_view             name;
  std::vector<BoardDeviceInfo> devices;
};

class SensorBoardManager {
public:
  static inline const SensorBoardInfo& getSensorBoardInfo(SensorBoardType type) { return sensorBoardDatabase.at(type); }

  // Per-board, per-device pose offset relative to the board center.
  static inline DevicePoseOffset getDevicePoseOffset(SensorBoardType board_type, const DeviceID& id) {
    const auto& board = sensorBoardDatabase.at(board_type);
    for (const auto& dev : board.devices) {
      if (dev.id.type == id.type && dev.id.index == id.index) {
        return dev.pose_offset;
      }
    }
    // Fallback: zero offset if not found.
    return DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
  }

  // Optional helper to get the static info for a specific device instance.
  static inline const AnyDeviceInfo* getDeviceInfo(SensorBoardType board_type, const DeviceID& id) {
    const auto& board = sensorBoardDatabase.at(board_type);
    for (const auto& dev : board.devices) {
      if (dev.id.type == id.type && dev.id.index == id.index) {
        return &dev.info;
      }
    }
    return nullptr;
  }

private:
  // Per-device-type static infos (shared across all boards).
  static inline const VL53L8CX_DeviceInfo VL53L8CX_INFO{
    "ST VL53L8CX", 45.0, 45.0, 8, 8, 15.0
  };

  static inline const HTPA32_DeviceInfo HTPA32_INFO{
    "Heimann HTPA32", 32, 32, 15.0
  };

  static inline const std::unordered_map<SensorBoardType, SensorBoardInfo> sensorBoardDatabase = {
    {
      SensorBoardType::Headlight,
      {
        "Headlight",
        {
          // VL53L8CX device instance
          BoardDeviceInfo{
            DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            VL53L8CX_INFO
          },
          // HTPA32 device instance
          BoardDeviceInfo{
            DeviceID{ DeviceType::HTPA32, "Thermal Sensor", 0 },
            DevicePoseOffset{ { 0.013, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            HTPA32_INFO
          },
          // WS2812b lights
          BoardDeviceInfo{
            DeviceID{ DeviceType::WS2812b, "Light", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            WS2812b_DeviceInfo{ "WS2812b (11 LEDs)", 11 }
          },
        }
      }
    },
    {
      SensorBoardType::Taillight,
      {
        "Taillight",
        {
          BoardDeviceInfo{
            DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            VL53L8CX_INFO
          },
          // No HTPA32 device on Taillight
          BoardDeviceInfo{
            DeviceID{ DeviceType::WS2812b, "Light", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            WS2812b_DeviceInfo{ "WS2812b (8 LEDs)", 8 }
          },
        }
      }
    },
    {
      SensorBoardType::Sidepanel,
      {
        "Sidepanel",
        {
          BoardDeviceInfo{
            DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            VL53L8CX_INFO
          },
          BoardDeviceInfo{
            DeviceID{ DeviceType::WS2812b, "Light", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            WS2812b_DeviceInfo{ "WS2812b (2 LEDs)", 2 }
          },
        }
      }
    },
    {
      SensorBoardType::Minipanel,
      {
        "Minipanel",
        {
          BoardDeviceInfo{
            DeviceID{ DeviceType::VL53L8CX, "Tof Sensor", 0 },
            DevicePoseOffset{ { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
            VL53L8CX_INFO
          },
          // No HTPA32 or WS2812b devices on Minipanel (for now)
        }
      }
    },
    {
      SensorBoardType::Undefined,
      {
        "Unknown",
        {
          // No devices defined
        }
      }
    }
  };
};

} // namespace device

} // namespace eduart