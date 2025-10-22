#pragma once

#include <stdexcept>
#include <string_view>

namespace eduart {

namespace sensor {

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

struct TofSensorInfo {
  std::string_view name;
  double fov_x;
  double fov_y;
  int res_x;
  int res_y;
  double max_rate;
};

struct ThermalSensorInfo {
  std::string_view name;
  int res_x;
  int res_y;
  double max_rate;
};

struct LedLightInfo {
  std::string_view name;
  unsigned int count;
};

struct SensorBoardInfo {
  std::string_view name;
  TofSensorInfo tof;
  ThermalSensorInfo thermal;
  LedLightInfo leds;
};

class SensorBoardManager {
public:
  static inline TofSensorInfo getToFSensorInfo(TofType tofType) {
    switch (tofType) {
    case TofType::None:
      return tof_none;
    case TofType::VL53L8:
      return tof_vl53l8;
    default:
      throw std::invalid_argument("Unknown ToF sensor type");
    }
  }

  static inline ThermalSensorInfo getThermalSensorInfo(ThermalType thermalType) {
    switch (thermalType) {
    case ThermalType::None:
      return thermal_none;
    case ThermalType::HTPA32:
      return thermal_htpa32;
    default:
      throw std::invalid_argument("Unknown thermal sensor type");
    }
  }

  static inline SensorBoardInfo getSensorBoardInfo(SensorBoardType boardType) {
    switch (boardType) {
    case SensorBoardType::Unknown:
      return board_unknown;

    case SensorBoardType::Headlight:
      return board_headlight;

    case SensorBoardType::Taillight:
      return board_taillight;

    case SensorBoardType::Sidepanel:
      return board_sidepanel;

    case SensorBoardType::Minipanel:
      return board_minipanel;

    default:
      throw std::invalid_argument("Unknown sensor board type");
    }
  }

private:
  static constexpr TofSensorInfo tof_none    = { "none", 0.0, 0.0, 0, 0, 0.0 };
  static constexpr TofSensorInfo tof_unknown = { "n.a.", 0.0, 0.0, 0, 0, 0.0 };
  static constexpr TofSensorInfo tof_vl53l8  = { "ST VL53L8CX", 45.0, 45.0, 8, 8, 15.0 };

  static constexpr ThermalSensorInfo thermal_none    = { "none", 0, 0, 0.0 };
  static constexpr ThermalSensorInfo thermal_unknown = { "n.a.", 0, 0, 0.0 };
  static constexpr ThermalSensorInfo thermal_htpa32  = { "Heimann HTPA32", 32, 32, 15.0 };

  static constexpr LedLightInfo leds_none      = { "none", 0 };
  static constexpr LedLightInfo leds_unknown   = { "n.a.", 0 };
  static constexpr LedLightInfo leds_headlight = { "WS2812b", 11 };
  static constexpr LedLightInfo leds_taillight = { "WS2812b", 8 };
  static constexpr LedLightInfo leds_sidepanel = { "WS2812b", 2 };

  static constexpr SensorBoardInfo board_unknown   = { "Unknown", tof_unknown, thermal_unknown, leds_unknown };
  static constexpr SensorBoardInfo board_headlight = { "Headlight", tof_vl53l8, thermal_htpa32, leds_headlight };
  static constexpr SensorBoardInfo board_taillight = { "Taillight", tof_vl53l8, thermal_none, leds_taillight };
  static constexpr SensorBoardInfo board_sidepanel = { "Sidepanel", tof_vl53l8, thermal_none, leds_sidepanel };
  static constexpr SensorBoardInfo board_minipanel = { "Minipanel", tof_vl53l8, thermal_none, leds_none };
};

}

}