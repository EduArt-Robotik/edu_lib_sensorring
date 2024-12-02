#include "TofSensor.hpp"
#include <cstring>
#include <algorithm>

namespace eduart{

namespace sensor{

TofSensor::TofSensor(TofSensorParams params, com::ComInterface* interface, std::size_t idx) :
    BaseSensor(interface, com::ComEndpoint("tof" + std::to_string(idx) + "_data"), idx, params.enable),
    _rot_m(math::MiniMath::rotation_matrix_from_euler_degrees(params.rotation)),
    _params(params){

    _rx_buffer_offset = 0;
    std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
}

TofSensor::~TofSensor(){

}

const TofSensorParams& TofSensor::getParams() const{
    return _params;
}

const measurement::TofMeasurement* TofSensor::getLatestMeasurement() const{
    return &_latest_measurement;
}

const measurement::TofMeasurement* TofSensor::getLatestMeasurement(SensorState &error) const{
    error = _error;
    return &_latest_measurement;
}

void TofSensor::onClearDataFlag(){
    std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
    _rx_buffer_offset = 0;
}

void TofSensor::canCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data){
    std::size_t msg_size = data.size();

    // point data msg
    if(msg_size == 48){
        if((_rx_buffer_offset + msg_size) <= (int) sizeof(_rx_buffer)){
            std::copy_n(data.begin(), msg_size, (uint8_t*)&_rx_buffer + _rx_buffer_offset);
            _rx_buffer_offset += msg_size;
            //_new_data_in_buffer_flag = true;
            _new_data_available_flag = false;

            // got all 4 raw data messages
            if(_rx_buffer_offset >= sizeof(_rx_buffer)){
                _new_data_in_buffer_flag = true;
            }
        }else{
            _error = SensorState::ReceiveError;
        }
    
    // transmission complete message
    } else if(msg_size == 2){
        if(_new_data_in_buffer_flag){
            _latest_measurement = processMeasurement(data[1], _rx_buffer, TOF_RESOLUTION);
            _new_data_in_buffer_flag = false;
            _new_measurement_ready_flag = true;
        }
        
    // data available message
    } else if(msg_size==1){
        _new_data_available_flag = true;
    }
}

const measurement::TofMeasurement TofSensor::processMeasurement(int frame_id, uint8_t* data, int len) const{
    measurement::TofMeasurement measurement;
    measurement.reserve(TOF_RESOLUTION);
    measurement.frame_id = frame_id;

    uint16_t distance_raw = 0;
    uint16_t sigma_raw = 0;
    float point_distance = 0;
    float point_sigma = 0;

    int point_idx = 0;

    for(int i=0; i<len; i++){
        distance_raw    = (*((uint32_t*)(data + i*3)) >> 10) & 0x3FFF; // 14 bit
        sigma_raw       = (*((uint32_t*)(data + i*3)) >>  0) & 0x03FF; // 10 bit

        if(distance_raw != 0){
            point_distance  = (float)distance_raw  /  4.0F / 1000.0F; // Factor 4 for fixed point conversion, Factor 1000 from mm to m
            point_sigma     = (float)sigma_raw     / 128.0 / 1000.0F; // Factor 128 for fixed point conversion, Factor 1000 from mm to m

            math::Vector3 point;
            point.x() = point_distance * lut_tan_x[i] * (-1.0);
            point.y() = point_distance * lut_tan_y[i];
            point.z() = point_distance;

            measurement.point_distance.push_back(point_distance);
            measurement.point_sigma.push_back(point_sigma);
            measurement.point_data.push_back(point);
            point_idx++;
        }
    }

    measurement.size = point_idx;
    measurement.point_data_transformed = transformPointCloud(measurement.point_data, _rot_m, _params.translation);

    return measurement;
}

std::vector<math::Vector3> TofSensor::transformPointCloud(const measurement::PointCloud& point_data, const math::Matrix3 rotation, const math::Vector3 translation){
    const unsigned int size = point_data.size();

    std::vector<math::Vector3> point_data_transformed;
    point_data_transformed.reserve(size);

    for(unsigned int i = 0; i<size; i++){
        math::Vector3 transformed_point = (rotation * point_data[i]) + translation;
        point_data_transformed.push_back(transformed_point);
    }
    
    return point_data_transformed;
}

measurement::TofMeasurement TofSensor::combineTofMeasurements(const std::vector<const measurement::TofMeasurement*>& measurements_vec){
    measurement::TofMeasurement combined_measurement;

    unsigned int size = 0;
    for(auto measurement : measurements_vec){
        size += measurement->size;
    }

    // preallocate memory
    combined_measurement.size = size;
    combined_measurement.point_distance.reserve(size);
    combined_measurement.point_sigma.reserve(size);
    combined_measurement.point_data.reserve(size);
    combined_measurement.point_data_transformed.reserve(size);

    for(auto measurement : measurements_vec){
        combined_measurement.point_distance.insert(combined_measurement.point_distance.end(), measurement->point_distance.begin(), measurement->point_distance.end());
        combined_measurement.point_sigma.insert(combined_measurement.point_sigma.end(), measurement->point_sigma.begin(), measurement->point_sigma.end());
        combined_measurement.point_data.insert(combined_measurement.point_data.end(), measurement->point_data.begin(), measurement->point_data.end());
        combined_measurement.point_data_transformed.insert(combined_measurement.point_data_transformed.end(), measurement->point_data_transformed.begin(), measurement->point_data_transformed.end());
    }

    return combined_measurement;
}

}

}