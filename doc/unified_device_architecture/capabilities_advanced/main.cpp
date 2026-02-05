#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "LightDevice.hpp"
#include "TemperatureSensorDevice.hpp"

int main() {
  std::vector<std::unique_ptr<IDevice> > devices;
  devices.emplace_back(std::make_unique<LightDevice>());
  devices.emplace_back(std::make_unique<TemperatureSensorDevice>());

  for (size_t i = 0; i < devices.size(); ++i) {
    IDevice* dev = devices[i].get();
    std::cout << "Device[" << i << "]:\n";

    // Print capability names and types
    std::cout << "  Capabilities:\n";
    for (auto const& info : dev->capabilities()) {
      std::cout << "    - name=\"" << info.second << "\" type=\"" << info.first.name() << "\"\n";
    }

    // Example pattern: use non-throwing invoke and handle std::nullopt
    if (auto opt = dev->invoke<TurnOn>({})) {
      std::cout << "  TurnOn (non-throwing) -> ok=" << std::boolalpha << opt->ok << "\n";
    } else {
      std::cout << "  TurnOn not supported (non-throwing)\n";
    }

    // Or use throwing variant try_invoke (thin wrapper)
    try {
      auto res = dev->try_invoke<SetBrightness>(SetBrightness::Request{ 75 });
      std::cout << "  SetBrightness (sync, throwing) -> " << res.level << "\n";
    } catch (const CapabilityNotSupported& e) {
      std::cout << "  SetBrightness not supported: " << e.what() << "\n";
    }

    // Async example: non-throwing invoke_async
    if (auto fut_opt = dev->invoke_async<SetBrightness>(SetBrightness::Request{ 20 })) {
      std::cout << "  SetBrightness (async, non-throwing) -> waiting...\n";
      auto result = fut_opt->get();
      std::cout << "  SetBrightness (async) -> " << result.level << "\n";
    } else {
      std::cout << "  SetBrightness async not supported (non-throwing)\n";
    }

    // ReadTemperature (sync + async) and Calibrate
    if (auto temp_opt = dev->invoke<ReadTemperature>({})) {
      std::cout << "  ReadTemperature (sync) -> " << temp_opt->value << " C\n";
    } else {
      std::cout << "  ReadTemperature not supported (non-throwing)\n";
    }

    try {
      auto fut = dev->try_invoke_async<ReadTemperature>({});
      std::cout << "  ReadTemperature (async) -> waiting...\n";
      auto temp2 = fut.get();
      std::cout << "  ReadTemperature (async) -> " << temp2.value << " C\n";
    } catch (const CapabilityNotSupported& e) {
      std::cout << "  ReadTemperature async not supported: " << e.what() << "\n";
    }

    // Calibrate (throwing)
    try {
      auto ok = dev->try_invoke<Calibrate>(Calibrate::Request{ 0.5 });
      std::cout << "    Calibrate -> ok=" << std::boolalpha << ok.ok << "\n";
      auto after = dev->try_invoke<ReadTemperature>({});
      std::cout << "    New temperature -> " << after.value << " C\n";
    } catch (const CapabilityNotSupported& e) {
      std::cout << "    Calibrate not supported: " << e.what() << "\n";
    }

    std::cout << "\n";
  }

  return 0;
}