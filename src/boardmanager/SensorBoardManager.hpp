#pragma once

#include <string_view>
#include <unordered_map>

#include "Math.hpp"

namespace eduart {

namespace sensor {

// Numbers match the definition in the sensor board firmware
enum class SensorBoardType {
  Headlight = 0,
  Taillight = 1,
  Sidepanel = 2,
  Minipanel = 3,
  Unknown   = 4
};

enum class TofType {
  None,
  VL53L8
};

enum class ThermalType {
  None,
  HTPA32
};

enum class LightType {
  None,
  WS2812b_11,
  WS2812b_8,
  WS2812b_2
};

struct TofSensorInfo {
  std::string_view name;
  double fov_x;
  double fov_y;
  int res_x;
  int res_y;
  double max_rate;
  math::Vector3 board_center_translation_offset;
  math::Vector3 board_center_rotation_offset;
};

struct ThermalSensorInfo {
  std::string_view name;
  int res_x;
  int res_y;
  double max_rate;
  math::Vector3 board_center_translation_offset;
  math::Vector3 board_center_rotation_offset;
};

struct LightInfo {
  std::string_view name;
  unsigned int count;
};

struct SensorBoardInfo {
  std::string_view name;
  TofSensorInfo tof;
  ThermalSensorInfo thermal;
  LightInfo leds;
};

class SensorBoardManager {
public:
  static inline LightInfo geLightInfo(LightType type) { return lightDatabase.at(type); }

  static inline TofSensorInfo getToFSensorInfo(TofType type) { return tofSensorDatabase.at(type); }

  static inline ThermalSensorInfo getThermalSensorInfo(ThermalType type) { return thermalSensorDatabase.at(type); }

  static inline SensorBoardInfo getSensorBoardInfo(SensorBoardType type) { return sensorBoardDatabase.at(type); }

private:
  static inline const std::unordered_map<LightType, LightInfo> lightDatabase = {
    { LightType::None,       { "none", 0 }     },
    { LightType::WS2812b_11, { "WS2812b", 11 } },
    { LightType::WS2812b_8,  { "WS2812b", 8 }  },
    { LightType::WS2812b_2,  { "WS2812b", 2 }  }
  };

  static inline const std::unordered_map<TofType, TofSensorInfo> tofSensorDatabase = {
    { TofType::None,   { "none", 0.0, 0.0, 0, 0, 0.0, { 0, 0, 0 }, { 0, 0, 0 } }           },
    { TofType::VL53L8, { "ST VL53L8CX", 45.0, 45.0, 8, 8, 15.0, { 0, 0, 0 }, { 0, 0, 0 } } }
  };

  static inline const std::unordered_map<ThermalType, ThermalSensorInfo> thermalSensorDatabase = {
    { ThermalType::None,   { "none", 0, 0, 0.0, { 0, 0, 0 }, { 0, 0, 0 } }                  },
    { ThermalType::HTPA32, { "Heimann HTPA32", 32, 32, 15.0, { 0.013, 0, 0 }, { 0, 0, 0 } } }
  };

  static inline const std::unordered_map<SensorBoardType, SensorBoardInfo> sensorBoardDatabase = {
    { SensorBoardType::Unknown,   { "Unknown", tofSensorDatabase.at(TofType::None), thermalSensorDatabase.at(ThermalType::None), lightDatabase.at(LightType::None) }             },
    { SensorBoardType::Headlight, { "Headlight", tofSensorDatabase.at(TofType::VL53L8), thermalSensorDatabase.at(ThermalType::HTPA32), lightDatabase.at(LightType::WS2812b_11) } },
    { SensorBoardType::Taillight, { "Taillight", tofSensorDatabase.at(TofType::VL53L8), thermalSensorDatabase.at(ThermalType::None), lightDatabase.at(LightType::WS2812b_8) }    },
    { SensorBoardType::Sidepanel, { "Sidepanel", tofSensorDatabase.at(TofType::VL53L8), thermalSensorDatabase.at(ThermalType::None), lightDatabase.at(LightType::WS2812b_2) }    },
    { SensorBoardType::Minipanel, { "Minipanel", tofSensorDatabase.at(TofType::VL53L8), thermalSensorDatabase.at(ThermalType::None), lightDatabase.at(LightType::None) }         }
  };
};

} // namespace sensor

} // namespace eduart