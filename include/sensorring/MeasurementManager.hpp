#pragma once

#include "Parameters.hpp"
#include "MeasurementObserver.hpp"

#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>


// Forward declaration
namespace ring{
    class SensorRing;
};


namespace manager{

// Forward declaration
enum class MeasurementState;

class MeasurementManager{
public:
    MeasurementManager(ManagerParams params);
    MeasurementManager(ManagerParams params, MeasurementObserver* observer);
    ~MeasurementManager();
    
    int measureSome();
    int startMeasuring();
    int stopMeasuring();

    void registerObserver(MeasurementObserver* observer);
    
    std::string printTopology() const;
    WorkerState getWorkerState() const;
    ManagerParams getParams() const;

    void enableTofMeasurement(bool state);
    void enableThermalMeasurement(bool state);
    bool stopThermalCalibration();
    bool startThermalCalibration(std::size_t window);

private:
    void init();
    void StateMachine();
    void StateMachineWorker();
    
    int  notifyToFData();
    int  notifyThermalData();
    void notifyState(const WorkerState state);
    
    WorkerState _manager_state;
    MeasurementState _measurement_state;
    ManagerParams _params;
    std::unique_ptr<ring::SensorRing> _sensor_ring;

    bool _tof_enabled;
    bool _thermal_enabled;
    bool _first_measurement;
    std::chrono::time_point<std::chrono::steady_clock>  _last_tof_measurement_timestamp_s;
    std::chrono::time_point<std::chrono::steady_clock>  _last_thermal_measurement_timestamp_s;

    bool _is_tof_throttled;
    bool _is_thermal_throttled;
    bool _thermal_measurement_flag;

    std::vector<MeasurementObserver*> _observer_vec;

    std::atomic<bool> _is_running;
    std::thread _worker_thread;

};

};