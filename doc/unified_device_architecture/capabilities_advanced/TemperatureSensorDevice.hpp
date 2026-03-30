#pragma once

#include <chrono>
#include <thread>

#include "IDevice.hpp"

// Sensor-specific capability types
struct ReadTemperature {
  struct Request {};
  struct Response {
    double value;
  }; // Celsius
};

struct Calibrate {
  struct Request {
    double offset;
  };
  struct Response {
    bool ok;
  };
};

// Temperature sensor: ReadTemperature (sync + async), Calibrate (sync)
struct TemperatureSensorDevice : IDevice, ICapability<ReadTemperature>, ICapabilityAsync<ReadTemperature>, ICapability<Calibrate> {
  TemperatureSensorDevice() {
    register_capability<ReadTemperature>("read_temperature");
    register_capability_async<ReadTemperature>("read_temperature_async"); // sensor can provide async reads
    register_capability<Calibrate>("calibrate");
  }

  // ReadTemperature - synchronous (fast cached read)
  ReadTemperature::Response invoke(const ReadTemperature::Request&) override { return ReadTemperature::Response{ last_temp }; }
  ReadTemperature::Response invoke(const ReadTemperature::Request&) const override { return ReadTemperature::Response{ last_temp }; }

  // ReadTemperature - asynchronous (simulate actual sensor sampling delay)
  std::future<ReadTemperature::Response> invoke_async(const ReadTemperature::Request&) override {
    return std::async(std::launch::async, [this]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(150)); // sampling delay
      last_temp += 0.01;
      return ReadTemperature::Response{ last_temp };
    });
  }
  std::future<ReadTemperature::Response> invoke_async(const ReadTemperature::Request&) const override {
    return std::async(std::launch::async, [this]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      return ReadTemperature::Response{ last_temp };
    });
  }

  // Calibrate (mutating)
  Calibrate::Response invoke(const Calibrate::Request& offset) override {
    last_temp += offset.offset;
    return Calibrate::Response{ true };
  }

private:
  double last_temp = 21.5;
};