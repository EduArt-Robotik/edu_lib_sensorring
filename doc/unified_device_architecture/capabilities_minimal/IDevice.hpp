#pragma once

#include <typeindex>
#include <vector>
#include <algorithm>
#include <type_traits>

// Templated capability interface a device may implement for a capability tag.
// A capability tag must provide nested types: Request and Response.
template<typename Cap>
struct ICapability {
    using Request = typename Cap::Request;
    using Response = typename Cap::Response;

    // Implementors provide this method.
    virtual Response invoke(const Request& req) = 0;
    virtual ~ICapability() = default;
};

// Base device type. Contains:
// - a pure-virtual capabilities() method that must be overridden by concrete devices
//   to list the capability tag types they implement (via std::type_index).
// - a type-safe supports<Cap>() helper that checks the capability list.
// - a try_invoke<Cap>() helper that uses dynamic_cast to forward the call when supported.
struct IDevice {
    virtual ~IDevice() = default;

    // Return the list of capability tag types implemented by this device.
    // Each entry should be `typeid(MyCapabilityTag)`.
    virtual std::vector<std::type_index> capabilities() const = 0;

    // Query whether this device implements capability Cap (compile-time type).
    template<typename Cap>
    bool supports() const {
        auto caps = capabilities();
        return std::any_of(caps.begin(), caps.end(),
                           [](const std::type_index& t) { return t == typeid(Cap); });
    }

    // Try to invoke capability Cap on this device.
    // - If the device implements ICapability<Cap>, the call is forwarded and true is returned.
    // - Otherwise, returns false and 'out' is untouched.
    //
    // Note: this is non-const because it forwards to a non-const invoke() on the capability.
    template<typename Cap>
    bool try_invoke(const typename Cap::Request& req, typename Cap::Response& out) {
        if (auto* cap = dynamic_cast<ICapability<Cap>*>(this)) {
            out = cap->invoke(req);
            return true;
        }
        return false;
    }
};