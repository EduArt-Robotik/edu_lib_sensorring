#pragma once

#include <string>
#include "CustomTypes.hpp"


/**
 * @enum LogVerbosity
 * @brief Verbosity levels of the logger output
 * @author Hannes Duske
 * @date 25.12.2024
 */
enum class LogVerbosity{
    Debug,
    Info,
    Warning,
    Error
};


/**
 * @enum WorkerState
 * @brief Health state of the sensorring state machine worker
 * @author Hannes Duske
 * @date 25.12.2024
 */
enum class WorkerState{
    Initialized,
    Running,
    Shutdown,
    Error
};


/**
 * @class MeasurementObserver
 * @brief Observer interface of the MeasurementManager class. Defines the callback methods that are triggered by the MeasurementManager. It is possible to implement only one or a selection of the callback methods.
 * @author Hannes Duske
 * @date 25.12.2024
 */
class MeasurementObserver{
    public:
    
        /**
         * Constructor
         */
        MeasurementObserver() = default;
        
        /**
         * Destructor
         */
        ~MeasurementObserver() = default;

        /**
         * Callback method for state changes of the state machine worker
         * @param[in] status the new status of the state machine worker
         */
        virtual void onStateChange(__attribute_maybe_unused__ const WorkerState status) {};

        /**
         * Callback method for the log output of the sensorring library
         * @param[in] verbosity verbosity level of the log message
         * @param[in] msg       log message string
         */
        virtual void onOutputLog(__attribute_maybe_unused__ const LogVerbosity verbosity, __attribute_maybe_unused__ const std::string msg) {};

        /**
         * Callback method for new Time-of-Flight sensor measurements. Returns one combined measurement from all sensors.
         * @param[in] measurement the most recent combined Time-of-Flight sensor measurements
         */
        virtual void onTofMeasurement(__attribute_maybe_unused__ const measurement::TofMeasurement measurement) {};
        
        /**
         * Callback method for new thermal sensor measurements. Returns one individual measurements of each thermal sensor.
         * @param[in] idx the index of the thermal sensor that that recorded the measurement. Index starts at zero (0) for the first thermal sensor and only counts active thermal sensors.
         * @param[in] measurement the most recent thermal sensor measurement of the specified sensor
         */
        virtual void onThermalMeasurement(__attribute_maybe_unused__ const std::size_t idx, __attribute_maybe_unused__ const measurement::ThermalMeasurement measurement) {};        
};