#include <stdexcept>

namespace SensorBoardManager{

enum BoardType{
    Undefined,
    Headlight,
    Taillight,
    Sidepanel,
    Minipanel
};

enum class TofType{
    None,
    VL53L8
};

enum class ThermalType{
    None,
    HTPA32
};

struct Tof{
    double fov_x;
    double fov_y;
    int res_x;
    int res_y;
    double max_rate;
};

struct Thermal {
    int res_x;
    int res_y;
    double max_rate;
};

struct Leds {
    unsigned int count;
};

struct SensorBoard
{
    Tof tof;
    Thermal thermal;    
    Leds leds;
};

constexpr Tof tof_none      = {0.0, 0.0, 0, 0, 0.0};
constexpr Tof tof_vl53l8    = {45.0, 45.0, 8, 8, 15.0};

constexpr Thermal thermal_none   = {0, 0, 0.0};
constexpr Thermal thermal_htpa32 = {32, 32, 15.0};

constexpr SensorBoard sensor_undefined = {tof_none,     thermal_none,   {0}};
constexpr SensorBoard sensor_headlight = {tof_vl53l8,   thermal_htpa32, {11}};
constexpr SensorBoard sensor_taillight = {tof_vl53l8,   thermal_none,   {8}};
constexpr SensorBoard sensor_sidepanel = {tof_vl53l8,   thermal_none,   {2}};
constexpr SensorBoard sensor_minipanel = {tof_vl53l8,   thermal_none,   {0}};

inline Tof getToFParameters(TofType tofType) {
    switch (tofType) {
        case TofType::None:
            return tof_none;
        case TofType::VL53L8:
            return tof_vl53l8;
        default:
            throw std::invalid_argument("Unknown ToF sensor type");
    }
}

inline Thermal getThermalParameters(ThermalType thermalType) {
    switch (thermalType) {
        case ThermalType::None:
            return thermal_none;
        case ThermalType::HTPA32:
            return thermal_htpa32;
        default:
            throw std::invalid_argument("Unknown thermal sensor type");
    }
}

inline SensorBoard getBoardParameters(BoardType boardType) {
    switch (boardType) {
        case BoardType::Undefined:
            return sensor_undefined;

        case BoardType::Headlight:
            return sensor_headlight;

        case BoardType::Taillight:
            return sensor_taillight;

        case BoardType::Sidepanel:
            return sensor_sidepanel;

        case BoardType::Minipanel:
            return sensor_minipanel;

        default:
            throw std::invalid_argument("Unknown sensor board type");
    }
};

}