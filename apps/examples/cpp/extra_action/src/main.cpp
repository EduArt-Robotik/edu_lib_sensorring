// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example receives measurements and prints the current measurement rate to the command line.
 * @date 2025-11-18
 */

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/hardware/ws2812b/WS2812b_Device.hpp>
#include <sensorring/manager/MeasurementManager.hpp>
#include <thread>

using namespace eduart;
using namespace std::chrono_literals;

static constexpr std::string_view INTERFACE_NAME   = "can0";
static constexpr com::InterfaceType INTERFACE_TYPE = com::InterfaceType::SOCKETCAN;

// Parameters for smooth color cycling of the WS2812b lights.

static constexpr float BRIGHTNESS = 0.2f;
static constexpr float STEP       = 0.15f;      // Speed of the color change
static constexpr float OFFSET_G   = 2.0943951f; // 120 degrees phase shift
static constexpr float OFFSET_B   = 4.1887902f; // 240 degrees phase shift

int main(int, char*[]) {
  std::cout << "===============================" << std::endl;
  std::cout << "Extra action sensorring example" << std::endl;
  std::cout << "===============================" << std::endl;
  std::cout << std::endl;

  manager::ManagerParams params;

  com::ComInterfaceID interface;
  interface.type = INTERFACE_TYPE;
  interface.name = INTERFACE_NAME;

  try {
    // Create SensorRing via factory with 2 boards on 1 bus
    ring::SensorRingFactory factory;
    factory.addInterface(interface);
    auto sensor_ring = factory.build(ring::ValidationMode::Relaxed);

    if (!sensor_ring) {
      std::cout << "Failed to create SensorRing. Exiting." << std::endl;
      return 1;
    }

    auto manager = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));

    // Subscribe to ToF device group for rate tracking
    std::atomic<bool> got_first       = false;
    std::atomic<unsigned int> counter = 0;
    auto tof_sub                      = manager->subscribeToDeviceGroup(device::DeviceType::VL53L8CX, [&got_first, &counter](const device::DeviceGroup&) {
      got_first = true;
      counter++;
    });

    // Start the measurements
    manager->startMeasuring();

    while (!got_first && manager->isMeasuring()) {
    }

    if (manager->isMeasuring()) {
      std::cout << std::endl << "Sensorring successfully initialized." << std::endl;
      std::cout << std::endl << "Start printing animation frames:" << std::endl;

      float phase     = 0.0f;
      auto last_print = std::chrono::steady_clock::now();
      while (manager->isMeasuring()) {
        // Advance phase and compute smooth RGB values from three sine waves.
        phase += STEP;
        auto to_channel = [](float value) {
          const float v = 0.5f * (std::sin(value) + 1.0f); // map [-1,1] -> [0,1]
          int c         = static_cast<int>(v * 255.0f + 0.5f);
          if (c < 0)
            c = 0;
          if (c > 255)
            c = 255;
          return static_cast<std::uint8_t>(c);
        };

        const auto red   = to_channel(phase) * BRIGHTNESS;
        const auto green = to_channel(phase + OFFSET_G) * BRIGHTNESS;
        const auto blue  = to_channel(phase + OFFSET_B) * BRIGHTNESS;

        // Update the light color via the extra action interface so it runs in the MeasurementManager context.
        manager->enqueueExtraAction([red, green, blue]() {
          device::WS2812b_Device::setLight(light::LightMode::FixedColor, red, green, blue);
        });

        if (std::chrono::steady_clock::now() - last_print > 1s) {
          std::cout << "Current frame: " << counter.load() << "\r" << std::flush;
          last_print = std::chrono::steady_clock::now();
        }
        std::this_thread::sleep_for(50ms);
      }

      manager->unsubscribe(tof_sub);

      // Stop the measurements
      manager->stopMeasuring();
    }
  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}