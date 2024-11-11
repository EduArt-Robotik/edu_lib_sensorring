#pragma once

#include "Parameters.hpp"
#include "CustomTypes.hpp"
#include "MeasurementObservers.hpp"

#include <vector>
#include <string>
#include <sstream>
#include <atomic>
#include <thread>
#include <chrono>


// Forward declaration
namespace sensorring{
    class SensorRing;
};


namespace measurementmanager{

// Forward declaration
enum class MeasurementState;

class MeasurementManager{
public:
    MeasurementManager(MeasurementManagerParams params);
    ~MeasurementManager();
    
    int measureSome();
    int startMeasuring();
    int stopMeasuring();

    void registerLogObserver(LogObserver* observer);
    void registerTofObserver(TofObserver* observer);
    void registerThermalObserver(ThermalObserver* observer);

    std::string printTopology();
    void enableTofMeasurement(bool state);
    void enableThermalMeasurement(bool state);
    void stopThermalCalibration();
    void startThermalCalibration(std::size_t window);

private:
    int publishToFData();
    int publishThermalData();
    void StateMachine();
    void StateMachineWorker();
    
    void log(const LogVerbosity verbosity, const std::string msg);
    void log(const LogVerbosity verbosity, const std::stringstream msg);
    
    MeasurementState _state;
    MeasurementManagerParams _params;
    std::unique_ptr<sensorring::SensorRing> _sensor_ring;

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