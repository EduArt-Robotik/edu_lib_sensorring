#pragma once

#include "IDevice.hpp"
#include <variant> // for std::monostate

// Light-specific capability types (defined in the device header)
struct TurnOn {
    using Request = std::monostate; // no args
    using Response = bool;          // success
};

struct SetBrightness {
    using Request = int;            // brightness 0..100
    using Response = int;           // new brightness
};

// Light device implementing its own capabilities
struct LightDevice : IDevice,
                     ICapability<TurnOn>,
                     ICapability<SetBrightness>
{
    bool is_on = false;
    int brightness = 0;

    // Return the list of capabilities this device supports.
    std::vector<std::type_index> capabilities() const override {
        return { typeid(TurnOn), typeid(SetBrightness) };
    }

    // TurnOn capability
    bool invoke(const TurnOn::Request&) override {
        is_on = true;
        return true;
    }

    // SetBrightness capability
    int invoke(const SetBrightness::Request& b) override {
        brightness = b;
        return brightness;
    }
};