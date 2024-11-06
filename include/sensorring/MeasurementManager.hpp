#pragma once

#include "CustomTypes.hpp"
#include "MeasurementObservers.hpp"

#include <vector>
#include <string>
#include <sstream>
#include <atomic>
#include <thread>
#include <chrono>


// Forward declaration
namespace SensorRing{
    class SensorRing;
};

namespace MeasurementManager{

struct MeasurementManagerParams{
    bool print_topology;
    double frequency_tof_hz;
    double frequency_thermal_hz;
    std::string tf_name;
};

enum class MeasurementState{
    init,
    reset_sensors,
    enumerate_sensors,
    sync_lights,
    get_eeprom,
    pre_loop_init,
    request_tof_measurement,
    fetch_tof_data,
    request_thermal_measurement,
    fetch_thermal_data,
    wait_for_data,
    throttle_measurement,
    error_handler,
    shutdown
};

class MeasurementManager{
public:
    MeasurementManager(MeasurementManagerParams params, std::unique_ptr<SensorRing::SensorRing> sensor_ring);
    ~MeasurementManager();

    void enableTofMeasurement(bool state);
    void enableThermalMeasurement(bool state);
    int measureSome();
    int startMeasuring();
    int stopMeasuring();

    void registerLogObserver(LogObserver* observer);
    void registerTofObserver(TofObserver* observer);
    void registerThermalObserver(ThermalObserver* observer);

private:
    int publishToFData();
    int publishThermalData();
    std::string printTopology();
    void StateMachineWorker();
    void StateMachine();

    void stopThermalCalibration();
    void startThermalCalibration(std::size_t window);

    void log(const LogVerbosity verbosity, const std::string msg);
    void log(const LogVerbosity verbosity, const std::stringstream msg);
    
    MeasurementState _state;
    MeasurementManagerParams _params;
    std::unique_ptr<SensorRing::SensorRing> _sensor_ring;

    int _error;
    bool _tof_enabled;
    bool _thermal_enabled;
    bool _first_measurement;
    std::chrono::time_point<std::chrono::steady_clock>  _last_tof_measurement_timestamp_s;
    std::chrono::time_point<std::chrono::steady_clock>  _last_thermal_measurement_timestamp_s;

    bool _is_tof_throttled;
    bool _is_thermal_throttled;
    bool _thermal_measurement_flag;

    std::vector<LogObserver*> _log_observer_vec;
    std::vector<TofObserver*> _tof_observer_vec;
    std::vector<ThermalObserver*> _thermal_observer_vec;

    std::atomic<bool> _is_running;
    std::thread _worker_thread;

}; // class MeasurementManager

}; // namespace MeasurementManager