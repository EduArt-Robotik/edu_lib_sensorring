#pragma once

#include <string>
#include <vector>
#include "CustomTypes.hpp"

namespace sensor{
    struct LedLightParams{
//        int can_id = 0;
        int nr_of_leds = 0;
        SensorOrientation orientation = SensorOrientation::none;
    };

    struct ThermalSensorParams {
        int idx = 0;
        double t_min = 0;
        double t_max = 0;
//        double fov_x = 0;
//        double fov_y = 0;
//        unsigned int res_x = 0;
//        unsigned int res_y = 0;
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
        int idx = 0;
//        double fov_x = 0;
//        double fov_y = 0;
//        unsigned int res_x = 0;
//        unsigned int res_y = 0;
        math::Vector3 rotation = {0, 0, 0};
        math::Vector3 translation = {0, 0, 0};
    };

    struct SensorBoardParams{
        int idx = 0;
        //std::string interface_name;
        //std::shared_ptr<com::SocketCANFD> can_interface;
        // canid_t canid_tof;
        // canid_t canid_thermal;

        bool enable_tof;
        bool enable_thermal;

        TofSensorParams tof_params;
        ThermalSensorParams thermal_params;
        LedLightParams led_params;
    };

} //namespace Sensor


namespace sensorbus{
    struct SensorBusParams{
        std::string interface_name;
       //std::shared_ptr<com::SocketCANFD> can_interface;
    //    canid_t canid_tof_status;
    //    canid_t canid_tof_request;
    //    canid_t canid_thermal_status;
    //    canid_t canid_thermal_request;
    //    canid_t canid_light;
    //    canid_t canid_broadcast;

        std::vector<sensor::SensorBoardParams> board_param_vec;
    };
}; // namespace SensorBus


namespace sensorring{
    struct SensorRingParams{
        std::string tf_name;
        std::vector<sensorbus::SensorBusParams> bus_param_vec;
    };
}; // namespace SensorRing


namespace measurementmanager{
    struct MeasurementManagerParams{
        bool print_topology;
        double frequency_tof_hz;
        double frequency_thermal_hz;
        sensorring::SensorRingParams ring_params;
    };
}; // namespace MeasurementManager