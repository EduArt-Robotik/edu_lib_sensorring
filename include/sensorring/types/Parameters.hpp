#pragma once

#include <string>
#include <vector>
#include "CustomTypes.hpp"

namespace sensor{
    enum class SensorOrientation{
		left,
		right,
		none
	};

    struct LightParams{
        int nr_of_leds = 0;
        SensorOrientation orientation = SensorOrientation::none;
    };

    struct ThermalSensorParams {
        double t_min_deg_c = 0;
        double t_max_deg_c = 0;
        bool auto_min_max = false;
        bool use_eeprom_file = false;
        bool use_calibration_file = false;
        std::string eeprom_dir = "";
        std::string calibration_dir = "";
        math::Vector3 rotation = {0, 0, 0};
        math::Vector3 translation = {0, 0, 0};
        SensorOrientation orientation = SensorOrientation::none;
    };

    struct TofSensorParams {
        math::Vector3 rotation = {0, 0, 0};
        math::Vector3 translation = {0, 0, 0};
    };

    struct SensorBoardParams{
        std::size_t idx = 0;
        bool enable_tof;
        bool enable_thermal;

        LightParams led_params;
        TofSensorParams tof_params;
        ThermalSensorParams thermal_params;
    };

};


namespace bus{
    struct BusParams{
        std::string interface_name;
        std::vector<sensor::SensorBoardParams> board_param_vec;
    };
};


namespace ring{
    struct RingParams{
        std::string tf_name;
        std::vector<bus::BusParams> bus_param_vec;
    };
};


namespace manager{
    struct ManagerParams{
        bool print_topology;
        double frequency_tof_hz;
        double frequency_thermal_hz;
        ring::RingParams ring_params;
    };
};