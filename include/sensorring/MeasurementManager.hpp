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

/**
 * @class MeasurementManager
 * @brief Meta class that handles the timing, triggering and processing of sensor measurements. Internally it runs a looping state machine.
 * @author Hannes Duske
 * @date 25.12.2024
 */
class MeasurementManager{
public:
    /**
     * Constructor
     * @param[in] params Parameter structure for the MeasurementManager
     */
    MeasurementManager(ManagerParams params);

    /**
     * Constructor
     * @param[in] params Parameter structure for the MeasurementManager
     * @param[in] observer Observer that is automatically registered before any other internal action that no callbacks are missed.
     */
    MeasurementManager(ManagerParams params, MeasurementObserver* observer);

    /**
     * Destructor
     */
    ~MeasurementManager();
    
    /**
     * Run one processing cycle of the state machine worker
     * @return error code
     */
    bool measureSome();

    /**
     * Start running the state machine worker loop in a thread
     * @return error code
     */
    bool startMeasuring();

    /**
     * Stop the state machine worker loop and close the thread
     * @return error code
     */
    bool stopMeasuring();

    /**
     * Register an observer with the MeasurementManager object
     * @param[in] observer Observer that is registered and gets notified on future events
     */
    void registerObserver(MeasurementObserver* observer);
    
    /**
     * Get a string representation of the topology of the connected sensors
     * @return Formatted string describing the topology
     */
    std::string printTopology() const;

    /**
     * Get the health status of the state machine
     * @return Current worker state
     */
    WorkerState getWorkerState() const;

    /**
     * Get the parameters with which the MeasurementManager was initialized
     * @return Initial parameter struct
     */
    ManagerParams getParams() const;

    /**
     * Enable or disable the Time-of-Flight sensor measurements
     * @param[in] state enable signal
     */
    void enableTofMeasurement(bool state);

    /**
     * Enable or disable the thermal sensor measurements
     * @param[in] state enable signal
     */
    void enableThermalMeasurement(bool state);

    /**
     * Stop any ongoing thermal calibration
     * @return error code
     */
    bool stopThermalCalibration();

    /**
     * Start a thermal calibration
     * @param[in] window number of thermal frames used for averaging
     * @return error code
     */
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