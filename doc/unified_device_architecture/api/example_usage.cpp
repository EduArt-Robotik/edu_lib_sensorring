// Minimal usage examples including the streamlined readSnapshot overloads
#include "sensor_api.h"
#include <iostream>

using namespace sensorlib;

int main() {
    Manager mgr;

    // --- Discovery ---
    auto devs = mgr.listDevices();
    for (auto &d : devs) {
        std::cout << "known device: " << d.name
                  << " kind=" << (d.kind==DeviceKind::SENSOR ? "sensor":"actor")
                  << "\n";
    }

    // --- Typed single-sensor subscribe (preferred for C++ users) ---
    auto tok1 = mgr.subscribeSensor<SensorType::TEMPERATURE>("room1",
        [](const TemperatureMeasurement& m) {
            std::cout << "[typed] " << m.id.name << " = " << m.value
                      << " " << (m.unit ? *m.unit : "") << "\n";
        });

    // --- Read a sensor snapshot with ergonomic overload ---
    if (auto tempSnap = mgr.readSnapshot<SensorType::TEMPERATURE>("room1")) {
        std::cout << "Snapshot: " << tempSnap->id.name << " = " << tempSnap->value << "\n";
    } else {
        std::cout << "No temperature snapshot available for room1\n";
    }

    // -------------------------
    // Writing to an actor (control)
    // -------------------------
    mgr.sendCommand<ActorType::MOTOR>("left-wheels", 0.75);

    // --- Read an actor snapshot with ergonomic overload (if supported by actor) ---
    if (auto motorState = mgr.readSnapshot<ActorType::MOTOR>("left-wheels")) {
        std::cout << "Motor state snapshot available for " << motorState->id.name
                  << " value=" << motorState->value << "\n";
    } else {
        std::cout << "No actor snapshot available for left-wheels\n";
    }

    // cleanup
    mgr.unsubscribe(tok1);

    return 0;
}