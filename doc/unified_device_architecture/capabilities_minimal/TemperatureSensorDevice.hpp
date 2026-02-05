#pragma once

#include "IDevice.hpp"
#include <variant> // for std::monostate

// Sensor-specific capability types (defined in the device header)
struct ReadTemperature {
    using Request = std::monostate; // no args
    using Response = double;        // temperature in C
};

struct Calibrate {
    using Request = double;         // offset
    using Response = bool;          // success
};

// Temperature sensor implementing its own capabilities
struct TemperatureSensorDevice : IDevice,
                                 ICapability<ReadTemperature>,
                                 ICapability<Calibrate>
{
    double last_temp = 21.5;

    // Return the list of capabilities this device supports.
    std::vector<std::type_index> capabilities() const override {
        return { typeid(ReadTemperature), typeid(Calibrate) };
    }

    // ReadTemperature capability
    double invoke(const ReadTemperature::Request&) override {
        return last_temp;
    }

    // Calibrate capability
    bool invoke(const Calibrate::Request& offset) override {
        last_temp += offset;
        return true;
    }
};