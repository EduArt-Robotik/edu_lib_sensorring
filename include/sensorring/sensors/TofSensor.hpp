#pragma once

#include <vector>
#include "math/MiniMath.hpp"
#include "BaseSensor.hpp"
#include "utils/CustomTypes.hpp"

#define TOF_RESOLUTION 64

namespace Sensor{

static const double lut_tan_x[] = {-0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578};
static const double lut_tan_y[] = {0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578};

struct TofSensorParams {
    int idx = 0;
    double fov_x = 0;
    double fov_y = 0;
    unsigned int res_x = 0;
    unsigned int res_y = 0;
    mm::Vector3 rotation = {0, 0, 0};
    mm::Vector3 translation = {0, 0, 0};
};

class TofSensor : public BaseSensor{
    public:
        TofSensor(TofSensorParams params, std::shared_ptr<edu::SocketCANFD> can_interface, canid_t canid, bool enable);
        ~TofSensor();
        
        const TofSensorParams& getParams() const;
        const Measurement::TofSensorMeasurement* getLatestMeasurement() const;
        const Measurement::TofSensorMeasurement* getLatestMeasurement(SensorState &error) const;

        void canCallback(const canfd_frame& frame) override;
        void onClearDataFlag() override;

        static std::vector<mm::Vector3> transformPointCloud(const std::vector<mm::Vector3>& point_data, const mm::Matrix3 rotation, const mm::Vector3 translation);
        static Measurement::TofSensorMeasurement combineTofMeasurements(const std::vector<const Measurement::TofSensorMeasurement*>& measurements_vec);
    private:

        const Measurement::TofSensorMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

        const mm::Matrix3 _rot_m;
        const TofSensorParams _params;
        Measurement::TofSensorMeasurement _latest_measurement;

        uint8_t _rx_buffer[TOF_RESOLUTION * 3];
        unsigned int _rx_buffer_offset;
};

};