#pragma once

#include <string>
#include "CustomTypes.hpp"


enum class LogVerbosity{
    Debug,
    Info,
    Warning,
    Error
};

enum class WorkerState{
    Initialized,
    Running,
    Shutdown,
    Error
};

class MeasurementObserver{
    public:
        MeasurementObserver() = default;
        ~MeasurementObserver() = default;

        virtual void onStateChange(__attribute_maybe_unused__ const WorkerState status) {};
        virtual void onOutputLog(__attribute_maybe_unused__ const LogVerbosity verbosity, __attribute_maybe_unused__ const std::string msg) {};
        virtual void onTofMeasurement(__attribute_maybe_unused__ const measurement::TofSensorMeasurement measurement) {};
        virtual void onThermalMeasurement(__attribute_maybe_unused__ const std::size_t idx, __attribute_maybe_unused__ const measurement::ThermalSensorMeasurement measurement) {};        
};