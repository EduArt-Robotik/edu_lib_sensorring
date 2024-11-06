#include "sensors/ThermalSensor.hpp"
#include "utils/FileManager.hpp"
#include "utils/Iron.hpp"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace Sensor{

ThermalSensor::ThermalSensor(ThermalSensorParams params, std::shared_ptr<edu::SocketCANFD> can_interface, canid_t canid, bool enable) :
    BaseSensor(can_interface, canid, enable),
    _params(params){

    _rx_buffer_offset = 0;
    memset(_rx_buffer, 0, sizeof(_rx_buffer));

    _vdd = 0;
    _ptat = 0;

    _got_eeprom                 = false;
    _got_calibration            = false;
    _calibration_active         = false;
    _calibration_count_current  = 0;
    _calibration_count_goal     = 0;
    _calibration_average        = 0;

    _eeprom_filename        = "sensor" + std::to_string(_params.idx) + "_hpta32_eeprom.bin";
    _calibration_filename   = "sensor" + std::to_string(_params.idx) + "_hpta32_calibration.txt";
    
    if(_params.use_eeprom_file){
        _got_eeprom = FileManager::StructHandler<HeimannSensor::HTPA32Eeprom>::readStructFromFile(_params.eeprom_dir, _eeprom_filename, _eeprom);
    }
    
    if(_params.use_calibration_file){
        _got_calibration = FileManager::ArrayHandler<double, NUMBER_OF_PIXEL>::readArrayFromFile(_params.calibration_dir, _calibration_filename, _calibration_image.data);
        _calibration_average = _calibration_image.avg();
    }
};

ThermalSensor::~ThermalSensor(){
    
};

const ThermalSensorParams& ThermalSensor::getParams() const{
    return _params;
};

const Measurement::GrayscaleImage* ThermalSensor::getLatestGrayscaleImage() const{    
    return &_latest_grayscale_image;
}

const Measurement::GrayscaleImage* ThermalSensor::getLatestGrayscaleImage(SensorState &error) const{    
    error = _error;
    return &_latest_grayscale_image;
}

const Measurement::FalseColorImage* ThermalSensor::getLatestFalseColorImage() const{    
    return &_latest_falsecolor_image;
}

const Measurement::FalseColorImage* ThermalSensor::getLatestFalseColorImage(SensorState &error) const{   
    error = _error; 
    return &_latest_falsecolor_image;
}

const Measurement::ThermalSensorMeasurement* ThermalSensor::getLatestMeasurement() const{
    return &_latest_measurement;
}

const Measurement::ThermalSensorMeasurement* ThermalSensor::getLatestMeasurement(SensorState &error) const{
    error = _error;
    return &_latest_measurement;
}

bool ThermalSensor::gotEEPROM() const{
    return _got_eeprom;
};

void ThermalSensor::readEEPROM(){
    if(!_got_eeprom){
        _got_eeprom = false;
        _rx_buffer_offset = 0;
    }
};

bool ThermalSensor::stopCalibration(){
    bool result = _calibration_active;
    _calibration_active = false;
    return result;
};

bool ThermalSensor::startCalibration(size_t window){
    if(!_calibration_active){
        _calibration_active         = true;
        _calibration_count_goal     = window;
        _calibration_count_current  = 0;
        return true;
    }else{
        return false;
    }
};

void ThermalSensor::onClearDataFlag(){
    memset(_rx_buffer, 0, sizeof(_rx_buffer));
    _rx_buffer_offset = 0;
};

void ThermalSensor::canCallback(const canfd_frame& frame){
    if(!_got_eeprom){
        
        // check if there is still data to be written
        if((_rx_buffer_offset + frame.len) < (int) sizeof(HeimannSensor::HTPA32Eeprom) + CANFD_MAX_DLEN){

            // have to account fot last few values of the eeprom because sizeof(ETPA32Eeprom) is not dividable by 64 ( = canfd msg length)
            int len = (int) sizeof(HeimannSensor::HTPA32Eeprom) - _rx_buffer_offset;
            if(len > frame.len) len = frame.len;
            
            std::copy_n((uint8_t*)&frame.data , len, (uint8_t*)&_eeprom + _rx_buffer_offset);
            _rx_buffer_offset += len;
            
            if(_rx_buffer_offset >= (int) sizeof(HeimannSensor::HTPA32Eeprom)){
                _got_eeprom = true;
                FileManager::StructHandler<HeimannSensor::HTPA32Eeprom>::saveStructToFile(_params.eeprom_dir, _eeprom_filename, _eeprom);
            }
        }

    }else{
        if(!_new_measurement_ready_flag){
            // vdd and ptat  message
            if(frame.len == 4){
                _vdd  = (uint16_t) (frame.data[0] << 0 | frame.data[1] << 8);
                _ptat = (uint16_t) (frame.data[2] << 0 | frame.data[3] << 8);

            // data message
            }else if(frame.len == CANFD_MAX_DLEN){
                if((_rx_buffer_offset + frame.len) <= (int) sizeof(_rx_buffer)){
                    std::copy_n((uint8_t*)&frame.data, frame.len, (uint8_t*)&_rx_buffer + _rx_buffer_offset);
                    _rx_buffer_offset += frame.len;

                    if(_rx_buffer_offset >= sizeof(_rx_buffer)){
                        _latest_measurement = processMeasurement(0, _rx_buffer, _eeprom, _vdd, _ptat, NUMBER_OF_PIXEL);

                        // calibration routine
                        if(_calibration_active){
                            if(_calibration_count_current < _calibration_count_goal){
                                _calibration_image += _latest_measurement.temp_data_deg_c;
                                _calibration_count_current++;
                            }
                            if(_calibration_count_current >= _calibration_count_goal){
                                _calibration_image /= _calibration_count_current;
                                _calibration_average = _calibration_image.avg();
                                _calibration_active = false;
                                _got_calibration = true;

                                if(_params.use_calibration_file){
                                    FileManager::ArrayHandler<double, NUMBER_OF_PIXEL>::saveArrayToFile(_params.calibration_dir, _calibration_filename, _calibration_image.data);
                                }
                            }
                        }
                        
                        // apply calibration
                        if(!_calibration_active && _got_calibration){
                            _latest_measurement.temp_data_deg_c -= _calibration_image;
                            _latest_measurement.temp_data_deg_c += _calibration_average;
                        }

                        if(_params.auto_min_max){
                            _latest_grayscale_image = convertToGrayscaleImage(_latest_measurement.temp_data_deg_c, _latest_measurement.min_deg_c, _latest_measurement.max_deg_c);
                        }else{
                            _latest_grayscale_image = convertToGrayscaleImage(_latest_measurement.temp_data_deg_c, _params.t_min, _params.t_max);
                        }

                        rotateLeftImage(_latest_grayscale_image);
                        _latest_falsecolor_image = convertToFalseColorImage(_latest_grayscale_image);
                        _new_measurement_ready_flag = true;
                    }
                }else{
                    _error = SensorState::ReceiveError;
                }
            }
        }
    }
};

const Measurement::ThermalSensorMeasurement ThermalSensor::processMeasurement(const uint8_t frame_id, const uint8_t* data, const HeimannSensor::HTPA32Eeprom& eeprom, const uint16_t vdd, const uint16_t ptat, const size_t len) const {
    uint16_t* offset_data    = (uint16_t*)(data + 0);   //  256 bytes of buffer are top offset values
    uint16_t* raw_pixel_data = (uint16_t*)(data + 512); // 2048 bytes of buffer are pixel values

    std::vector<double> buffer(len);
    
    Measurement::ThermalSensorMeasurement result;
    result.frame_id = frame_id;
    result.min_deg_c = 1e6;

    // ambient temperature
    float t_ambient        = _ptat * eeprom.ptat_gradient + eeprom.ptat_offset;
    result.t_ambient_deg_c = (t_ambient - 2732) / 10.0F;

    // process raw buffer to thermal image
    for(unsigned int i=0; i<len; i++){
        int idx = i % 128;
        if(i >= (len / 2)) idx += 128;
        
        // deserialize thermal measurements. Everything is little endian except the raw pixel values which are big endian.
        uint16_t raw_pixel  = (((uint8_t*)raw_pixel_data)[i*2 + 0] << 8) | (((uint8_t*)raw_pixel_data)[i*2 + 1] << 0);
        
        // thermal offsets
        buffer[i] = raw_pixel - ((double)(eeprom.th_gradient[i] * ptat) / pow(2, eeprom.grad_scale)) - eeprom.th_offset[i];

        // electrical offsets
        buffer[i] -= offset_data[idx];

        // The buffer can be used as an image from here on (after subtracting the electric offsets). The following steps are only to calculate temperatures in °C.
        
        // vdd compensation
        double vdd_comp_val1 = ((double)(_eeprom.vddcomp_gradient[idx] * ptat) / std::pow(2, _eeprom.vddsc_gradient) + _eeprom.vddcomp_offset[idx]) / std::pow(2, _eeprom.vddsc_offset);
        double vdd_comp_val2 = (vdd - _eeprom.vddth1 - ((double)(_eeprom.vddth2 - _eeprom.vddth1) / (_eeprom.ptat_th2 - _eeprom.ptat_th1)) * (ptat - _eeprom.ptat_th1));
        buffer[i] = buffer[i] - (vdd_comp_val1 * vdd_comp_val2);

        // look-up table and bilinear interpolation
        unsigned int table_col = 0;
        for (int j = 0; j < NROFTAELEMENTS; j++) {
            if (t_ambient > HeimannSensor::XTATemps[j]) {
            table_col = j;
            }
        }

        unsigned int table_row = buffer[i] + TABLEOFFSET;
        table_row = table_row >> ADEXPBITS;

        int dta = t_ambient - HeimannSensor::XTATemps[table_col];
        
        double vx = ((((int32_t)HeimannSensor::TempTable[table_row][table_col + 1] - (int32_t)HeimannSensor::TempTable[table_row][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)HeimannSensor::TempTable[table_row][table_col];
        double vy = ((((int32_t)HeimannSensor::TempTable[table_row + 1][table_col + 1] - (int32_t)HeimannSensor::TempTable[table_row + 1][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)HeimannSensor::TempTable[table_row + 1][table_col];
        buffer[i] = (uint32_t)((vy - vx) * ((int32_t)(buffer[i] + TABLEOFFSET) - (int32_t)HeimannSensor::YADValues[table_row]) / (int32_t)ADEQUIDISTANCE + (int32_t)vx);

        // apply global offset
        //buffer[i] += _eeprom.global_offset;

        // calculate temperature in °C

        result.temp_data_deg_c.data[i] = (buffer[i] - 2732.0F) / 10.0F;
        if(result.temp_data_deg_c.data[i] < result.min_deg_c) result.min_deg_c = result.temp_data_deg_c.data[i];
        if(result.temp_data_deg_c.data[i] > result.max_deg_c) result.max_deg_c = result.temp_data_deg_c.data[i];
    }

    return result;
};

const Measurement::GrayscaleImage ThermalSensor::convertToGrayscaleImage(const Measurement::TemperatureImage& temp_data_deg_c, const double t_min_deg_c, const double t_max_deg_c) const {
    Measurement::GrayscaleImage result;

    // pixel data with min - max scaling
    double delta_t = (t_max_deg_c - t_min_deg_c);
    if(delta_t != 0){
        for(unsigned int i=0; i<512; i++){
            double norm_val = ((temp_data_deg_c.data[i] - t_min_deg_c) / delta_t) * 255;
            result.data[i] = static_cast<uint8_t>(std::clamp(std::round(norm_val), 0.0, 255.0));
        }

        // rearrange lower half of the image. See Heimann HTPA32 Datasheet.
        for(unsigned int i=512; i<1024; i+=128){
            for(unsigned int j=0; j<128; j+=32){
                for(unsigned int k=0; k<32; k++){
                    double norm_val = (temp_data_deg_c.data[i + (96-j) + k] - t_min_deg_c) / delta_t * 255;
                    result.data[i+j+k] = static_cast<uint8_t>(std::clamp(std::round(norm_val), 0.0, 255.0));
                }
            }
        }
    }

    return result;
};

const Measurement::FalseColorImage ThermalSensor::convertToFalseColorImage(const Measurement::GrayscaleImage& image) const{
    Measurement::FalseColorImage color_image;

    // convert latest measurement to false color image
    for(size_t i=0; i<NUMBER_OF_PIXEL; i++){
        color_image.data[i][0] = ThermalPalette::Iron[image.data[i]][0];
        color_image.data[i][1] = ThermalPalette::Iron[image.data[i]][1];
        color_image.data[i][2] = ThermalPalette::Iron[image.data[i]][2];
    }

    return color_image;
};

void ThermalSensor::rotateLeftImage(Measurement::GrayscaleImage& image) const{
    if(_params.orientation == Sensor::SensorOrientation::left){
       std::reverse(image.data.begin(), image.data.end());
    }
}


};