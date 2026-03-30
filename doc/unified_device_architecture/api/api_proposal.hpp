// Minimal prototype public API - sensor_api.h
#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <functional>
#include <unordered_map>
#include <cstdint>

namespace sensorlib {

// Basic time stamp
using Stamp = std::chrono::system_clock::time_point;

// Device kinds
enum class DeviceKind { SENSOR, ACTOR };

// Sensor / Actor type enums (extend when adding new hardware kinds)
enum class SensorType { TEMPERATURE, MULTI /* add new sensor types here */ };
enum class ActorType  { MOTOR /* add new actor types here */ };

// Strongly-typed device identifier
// (Simple layout: both sensorType and actorType fields present; use the one matching `kind`.)
struct DeviceId {
    DeviceKind kind;
    SensorType sensorType; // valid when kind == DeviceKind::SENSOR
    ActorType  actorType;  // valid when kind == DeviceKind::ACTOR
    std::string name;      // instance name / unique id
};

// -------------------------
// Typed Measurement model
// -------------------------

// Generic templated measurement container with common header + typed payload
template<typename PayloadT>
struct Measurement {
    DeviceId id;
    Stamp timestamp;
    // small metadata
    std::optional<std::string> unit;
    double confidence = 1.0;
    PayloadT value;
};

// Concrete payload aliases
using TempPayload = double;
using MultiPayload = std::vector<int>;

// Concrete measurement types
using TemperatureMeasurement = Measurement<TempPayload>;
using MultiMeasurement       = Measurement<MultiPayload>;

// Variant of all concrete measurement types (update when new types are added)
using MeasurementVariant = std::variant<TemperatureMeasurement, MultiMeasurement>;

// -------------------------
// Capabilities (traits)
// -------------------------
// Map each SensorType / ActorType to compile-time types and capabilities.
// Add specializations for each new hardware type.

template<SensorType S> struct SensorCapabilities; // no default
template<> struct SensorCapabilities<SensorType::TEMPERATURE> {
    using measurement_t = TemperatureMeasurement;
    using payload_t     = TempPayload;
    static constexpr const char* unit() { return "degC"; }
    static constexpr bool supportsStreaming = true;
};
template<> struct SensorCapabilities<SensorType::MULTI> {
    using measurement_t = MultiMeasurement;
    using payload_t     = MultiPayload;
    static constexpr const char* unit() { return "items"; }
    static constexpr bool supportsStreaming = true;
};

template<ActorType A> struct ActorCapabilities; // no default
// Example actor capability specialization for MOTOR
template<> struct ActorCapabilities<ActorType::MOTOR> {
    using command_t = double; // e.g., speed
    // If actors publish a state snapshot, define the state_measurement_t:
    using state_measurement_t = Measurement<double>; // minimal example
    static constexpr const char* pretty_name() { return "motor"; }
};

// -------------------------
// Subscriber base-class API
// -------------------------

// Base-class subscriber: receives runtime MeasurementVariant instances
class Subscriber {
public:
    virtual ~Subscriber() = default;
    // Called for each new measurement that matches the subscription.
    virtual void onMeasurement(const MeasurementVariant& m) = 0;
};

// Optional convenience helper: register typed handlers on top of Subscriber
// Clients can subclass TypedSubscriber and call registerHandler<SensorType::...>(...)
class TypedSubscriber : public Subscriber {
public:
    using HandlerFn = std::function<void(const MeasurementVariant&)>;

    // register a typed handler for a compile-time sensor type
    template<SensorType S>
    void registerHandler(std::function<void(const typename SensorCapabilities<S>::measurement_t&)> h) {
        int key = static_cast<int>(S);
        handlers_[key] = [h = std::move(h)](const MeasurementVariant& mv) {
            if (auto p = std::get_if<typename SensorCapabilities<S>::measurement_t>(&mv)) {
                h(*p);
            }
        };
    }

    void onMeasurement(const MeasurementVariant& m) override {
        std::visit([this](auto const &mm) {
            int key = (mm.id.kind == DeviceKind::SENSOR) ? static_cast<int>(mm.id.sensorType)
                                                        : -1;
            auto it = handlers_.find(key);
            if (it != handlers_.end()) it->second(m);
        }, m);
    }

private:
    std::unordered_map<int, HandlerFn> handlers_;
};

// -------------------------
// Common Device base-class (public surface)
// -------------------------
class Device {
public:
    virtual ~Device() = default;
    virtual DeviceId id() const = 0;
    virtual std::string name() const = 0;
    // For sensors: last measurement snapshot (may be empty if not available yet)
    virtual std::optional<MeasurementVariant> lastMeasurement() const = 0;
    // For actors: implementations may optionally return last commanded state via lastMeasurement()
};

// -------------------------
// Manager public API
// -------------------------
using SubscriptionToken = uint64_t;

class Manager {
public:
    Manager();
    ~Manager();

    // Discovery: which devices are currently known/connected
    std::vector<DeviceId> listDevices() const;
    std::vector<DeviceId> listDevicesBySensorType(SensorType t) const;
    std::vector<DeviceId> listDevicesByActorType(ActorType t) const;

    // Read a snapshot (synchronous) - returns std::nullopt if not available / wrong kind
    std::optional<MeasurementVariant> readSnapshot(const DeviceId& id) const;

    // Convenience, ergonomic readSnapshot overloads:
    // Read sensor snapshot by type and name (returns concrete measurement_t)
    template<SensorType S>
    std::optional<typename SensorCapabilities<S>::measurement_t>
    readSnapshot(const std::string& name) const;

    // Read actor snapshot by actor type and name (returns actor-specific state measurement type)
    template<ActorType A>
    std::optional<typename ActorCapabilities<A>::state_measurement_t>
    readSnapshot(const std::string& name) const;

    // -------------------------
    // Subscription API (flexible)
    // -------------------------
    // 1) Subscribe to a single sensor (compile-time typed callback)
    template<SensorType S, typename Callback>
    SubscriptionToken subscribeSensor(const std::string& name, Callback&& cb);

    // 2) Subscribe to multiple sensors (list of names) of the same SensorType
    template<SensorType S, typename Callback>
    SubscriptionToken subscribeSensors(const std::vector<std::string>& names, Callback&& cb);

    // 3) Subscribe to all sensors of a SensorType (every current & future sensor of that type)
    template<SensorType S, typename Callback>
    SubscriptionToken subscribeAllSensors(Callback&& cb);

    // Base-class style subscription (runtime) - subscribe to one or more names
    // Manager does NOT take ownership of Subscriber pointer; caller must ensure lifetime.
    SubscriptionToken subscribeSubscriber(Subscriber* sub, const std::vector<std::string>& names);
    // Subscribe subscriber to all sensors of a given type
    SubscriptionToken subscribeSubscriberToAll(Subscriber* sub, SensorType t);

    // Unsubscribe
    void unsubscribe(SubscriptionToken token);

    // -------------------------
    // Actor control API (write commands to actors)
    // -------------------------
    // Send a command to a single actor (compile-time typed)
    template<ActorType A>
    void sendCommand(const std::string& actorName, const typename ActorCapabilities<A>::command_t& cmd);

private:
    // Private/internal plumbing (omitted)
    struct Impl;
    Impl* pImpl_;
};

// -------------------------
// Template method definitions (minimal stubs for the prototype header)
// -------------------------
// Note: real implementations live in Manager::Impl; here we provide no-op stubs to keep
// header self-contained for example compilation of client usage. The real library will
// implement behavior in a .cpp file.

inline Manager::Manager() : pImpl_(nullptr) {}
inline Manager::~Manager() {}

inline std::vector<DeviceId> Manager::listDevices() const { return {}; }
inline std::vector<DeviceId> Manager::listDevicesBySensorType(SensorType) const { return {}; }
inline std::vector<DeviceId> Manager::listDevicesByActorType(ActorType) const { return {}; }
inline std::optional<MeasurementVariant> Manager::readSnapshot(const DeviceId&) const { return std::nullopt; }

template<SensorType S>
std::optional<typename SensorCapabilities<S>::measurement_t>
Manager::readSnapshot(const std::string&) const {
    // stub: real impl must:
    // 1) find device by name
    // 2) check it's a sensor and its SensorType == S
    // 3) extract concrete measurement_t from stored MeasurementVariant and return it
    return std::nullopt;
}

template<ActorType A>
std::optional<typename ActorCapabilities<A>::state_measurement_t>
Manager::readSnapshot(const std::string&) const {
    // stub: real impl must:
    // 1) find device by name
    // 2) check it's an actor and its ActorType == A
    // 3) extract actor state snapshot if supported, or return nullopt
    return std::nullopt;
}

template<SensorType S, typename Callback>
SubscriptionToken Manager::subscribeSensor(const std::string&, Callback&&) { return 0; }

template<SensorType S, typename Callback>
SubscriptionToken Manager::subscribeSensors(const std::vector<std::string>&, Callback&&) { return 0; }

template<SensorType S, typename Callback>
SubscriptionToken Manager::subscribeAllSensors(Callback&&) { return 0; }

inline SubscriptionToken Manager::subscribeSubscriber(Subscriber*, const std::vector<std::string>&) { return 0; }
inline SubscriptionToken Manager::subscribeSubscriberToAll(Subscriber*, SensorType) { return 0; }

template<ActorType A>
void Manager::sendCommand(const std::string&, const typename ActorCapabilities<A>::command_t&) {
    // no-op stub; real implementation forwards command to the correct actor device.
}

} // namespace sensorlib