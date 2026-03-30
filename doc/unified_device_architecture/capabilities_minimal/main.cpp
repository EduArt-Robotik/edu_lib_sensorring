#include <iostream>
#include <memory>
#include <vector>
#include <typeinfo>

#include "LightDevice.hpp"
#include "TemperatureSensorDevice.hpp"

int main() {
    // Caller receives a vector of generic IDevice pointers (e.g., discovered at runtime)
    std::vector<std::unique_ptr<IDevice>> devices;
    devices.emplace_back(std::make_unique<LightDevice>());
    devices.emplace_back(std::make_unique<TemperatureSensorDevice>());

    for (size_t i = 0; i < devices.size(); ++i) {
        IDevice* dev = devices[i].get();
        std::cout << "Device[" << i << "]:\n";

        // Print available capabilities (runtime type_info names)
        std::cout << "  Capabilities:\n";
        for (auto& ci : dev->capabilities()) {
            std::cout << "    - " << ci.name() << "\n"; // .name() is implementation-defined
        }

        // Type-safe checks and invokes (requires including the capability tag types)
        if (dev->supports<TurnOn>()) {
            bool ok;
            dev->try_invoke<TurnOn>({}, ok);
            std::cout << "  -> TurnOn supported, result=" << std::boolalpha << ok << "\n";
        }

        if (dev->supports<SetBrightness>()) {
            int new_b;
            dev->try_invoke<SetBrightness>(75, new_b);
            std::cout << "  -> SetBrightness supported, new=" << new_b << "\n";
        }

        if (dev->supports<ReadTemperature>()) {
            double temp;
            dev->try_invoke<ReadTemperature>({}, temp);
            std::cout << "  -> ReadTemperature supported, value=" << temp << " C\n";

            if (dev->supports<Calibrate>()) {
                bool ok;
                dev->try_invoke<Calibrate>(0.5, ok);
                std::cout << "     Calibrate supported, ok=" << std::boolalpha << ok << "\n";
                double temp2;
                dev->try_invoke<ReadTemperature>({}, temp2);
                std::cout << "     New temperature=" << temp2 << " C\n";
            }
        }

        std::cout << "\n";
    }

    return 0;
}