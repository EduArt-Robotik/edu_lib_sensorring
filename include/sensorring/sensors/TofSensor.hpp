#pragma once

#include <vector>
#include "Math.hpp"
#include "BaseSensor.hpp"
#include "CustomTypes.hpp"
#include "Parameters.hpp"

#define TOF_RESOLUTION 64

namespace sensor{

static const double lut_tan_x[] = {-0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578, -0.3578, -0.2505, -0.1483, -0.0491, 0.0491, 0.1483, 0.2505, 0.3578};
static const double lut_tan_y[] = {0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.3578, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.2505, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.1483, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, 0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.0491, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.1483, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.2505, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578, -0.3578};

class TofSensor : public BaseSensor{
    public:
        TofSensor(TofSensorParams params, com::ComInterface* interface, std::size_t idx, bool enable);
        ~TofSensor();
        
        const TofSensorParams& getParams() const;
        const measurement::TofMeasurement* getLatestMeasurement() const;
        const measurement::TofMeasurement* getLatestMeasurement(SensorState &error) const;

        void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;
        void onClearDataFlag() override;

        static std::vector<math::Vector3> transformPointCloud(const std::vector<math::Vector3>& point_data, const math::Matrix3 rotation, const math::Vector3 translation);
        static measurement::TofMeasurement combineTofMeasurements(const std::vector<const measurement::TofMeasurement*>& measurements_vec);
    private:

        const measurement::TofMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

        const math::Matrix3 _rot_m;
        const TofSensorParams _params;
        measurement::TofMeasurement _latest_measurement;

        uint8_t _rx_buffer[TOF_RESOLUTION * 3];
        unsigned int _rx_buffer_offset;
};

}