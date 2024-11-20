#pragma once

#include <string>
#include "CustomTypes.hpp"


class TofObserver{
    public:
        TofObserver() = default;
        ~TofObserver() = default;

        virtual void onTofMeasurement(const measurement::TofSensorMeasurement measurement) = 0;


    private:


}; // class TofObserver


class ThermalObserver{
    public:
        ThermalObserver() = default;
        ~ThermalObserver() = default;

        virtual void onThermalMeasurement(const measurement::ThermalSensorMeasurement measurement) = 0;

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
        ~LogObserver() = default;

        virtual void onOutputLog(const LogVerbosity verbosity, const std::string msg) = 0;

    private:


}; // class ThermalObserver