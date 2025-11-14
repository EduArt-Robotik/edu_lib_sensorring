#pragma once

#include <chrono>
#include <atomic>

#include <sensorring/MeasurementClient.hpp>
#include <sensorring/logger/LoggerClient.hpp>

namespace eduart {

class MeasurementProxy : public manager::MeasurementClient, public logger::LoggerClient
{
public:

    /**
    * @brief Get the rate of measurements since the last call of this method
    * @return measurement rate in Hz
    */
    double getRate();

    /**
    * @brief Check if the client got the first measurement
    * @return true if the client already got the first measurement
    */
    bool gotFirstMeasurement();

    /**
    * @brief Implement onRawTofMeasurement callback method
    */
    void onRawTofMeasurement([[maybe_unused]] std::vector<measurement::TofMeasurement> measurement_vec) override;

    /**
    * @brief Implement onRawTofMeasurement callback method
    */
    void onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] std::string msg) override;


private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using toSeconds = std::chrono::duration<double>;

    TimePoint _lastQuery;
    std::atomic<bool> _init_flag;
    std::atomic<unsigned int> _counter;
};

}