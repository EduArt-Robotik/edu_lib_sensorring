// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   main.cpp
 * @author EduArt Robotik GmbH
 * @brief  This example demonstrates how to control WS2812b LEDs with a smooth color cycling animation using the Light interface.
 *         It runs in actuator-only mode without requiring any sensors to drive the manager loop.
 * @date 2025-11-18
 */

#include <chrono>
#include <cmath>
#include <iostream>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/device/light/Light.hpp>
#include <sensorring/interface/InterfaceParams.hpp>
#include <sensorring/logger/Logger.hpp>
#include <thread>
#include <vector>

using namespace eduart::sensorring;
using namespace std::chrono_literals;

// Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
static constexpr std::string_view CAN_INTERFACE_NAME = "eduart-can0";

// Default USBtingo interface (cross-platform, uses the first available USBtingo device)
static constexpr std::string_view USBTINGO_INTERFACE_NAME = "0";

// Parameters for smooth color cycling of the WS2812b lights.

static constexpr float BRIGHTNESS = 0.2f;
static constexpr float STEP       = 0.15f;      // Speed of the color change
static constexpr float OFFSET_G   = 2.0943951f; // 120 degrees phase shift
static constexpr float OFFSET_B   = 4.1887902f; // 240 degrees phase shift

int main(int, char*[]) {
  std::cout << "================================" << std::endl;
  std::cout << "Light control sensorring example" << std::endl;
  std::cout << "================================" << std::endl;
  std::cout << std::endl;

  com::SocketCanParams can_interface{ std::string(CAN_INTERFACE_NAME) };
  com::UsbTingoParams usbtingo_interface{ std::string(USBTINGO_INTERFACE_NAME) };

  try {
    // Subscribe to the logger first so that all messages from initialization onward are captured.
    // Using a lambda here (rather than binding a class method) ensures the subscription is active
    // before any other object is constructed.
    auto log_sub = logger::Logger::getInstance()->subscribe([](const logger::LogVerbosity verbosity, const std::string& msg) {
      if (verbosity > logger::LogVerbosity::Debug)
        std::cout << "[" << verbosity << "] " << msg << std::endl;
    });

    // Create SensorRing via factory auto-discovery
    SensorRingFactory factory;
    factory.addInterface(can_interface);
    factory.addInterface(usbtingo_interface);

    auto ring = factory.build();
   
    std::vector<device::Light*> lights;
    for (auto* dev : ring->getDevices()) {
      if (auto* lt = dynamic_cast<device::Light*>(dev))
        lights.push_back(lt);
    }
    
    if(lights.empty()) {
      std::cout << "No lights found in the SensorRing. Exiting." << std::endl;
      return 1;
    }

    std::cout << std::endl << "Start printing animation frames:" << std::endl;

    float phase              = 0.0f;
    auto last_print          = std::chrono::steady_clock::now();
    unsigned int frame_count = 0;

    while (true) {
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

      // Update the light color via the Light interface (applied in next state-machine cycle)
      device::Light::setAllLights(lights, device::LightMode::FixedColor, red, green, blue);

      frame_count++;
      if (std::chrono::steady_clock::now() - last_print > 1s) {
        std::cout << "Current frame: " << frame_count << "\r" << std::flush;
        last_print = std::chrono::steady_clock::now();
      }
      std::this_thread::sleep_for(100ms);
    }

  } catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  return 0;
}