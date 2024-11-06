#pragma once

#include <string>
#include "CustomTypes.hpp"


class TofObserver{
    public:
        TofObserver() = default;
        virtual ~TofObserver();

        virtual void onTofMeasurement(const Measurement::TofSensorMeasurement* measurement) = 0;


    private:


}; // class TofObserver


class ThermalObserver{
    public:
        ThermalObserver() = default;
        virtual ~ThermalObserver();

        virtual void onThermalMeasurement(const Measurement::ThermalSensorMeasurement* measurement) = 0;

    private:


}; // class ThermalObserver

enum class LogVerbosity{
    Debug,
    Info,
    Warning,
    Error
};

class LogObserver{
    public:
        LogObserver() = default;
        virtual ~LogObserver();

        virtual void onOutputLog(const LogVerbosity verbosity, const std::string msg) = 0;

    private:


}; // class ThermalObserver