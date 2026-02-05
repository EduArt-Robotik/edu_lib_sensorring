#pragma once

#include <chrono>
#include <thread>

#include "IDevice.hpp"

// Light-specific capability types (kept in the device header for now)
struct TurnOn {
  struct Request {};
  struct Response {
    bool ok;
  };
};

struct SetBrightness {
  struct Request {
    int level;
  }; // 0..100
  struct Response {
    int level;
  }; // new brightness
};

// Light device: implements TurnOn (sync) and SetBrightness (sync + async)
struct LightDevice : IDevice, ICapability<TurnOn>, ICapability<SetBrightness>, ICapabilityAsync<SetBrightness> {
  LightDevice() {
    // explicit registration calls (typed invokers created for each Cap)
    register_capability<TurnOn>("turn_on");
    register_capability<SetBrightness>("set_brightness");
    register_capability_async<SetBrightness>("set_brightness_async"); // async optional
  }

  // TurnOn (mutating)
  TurnOn::Response invoke(const TurnOn::Request&) override {
    is_on = true;
    return TurnOn::Response{ true };
  }
  // const variant (provided for const-correctness)
  TurnOn::Response invoke(const TurnOn::Request& /*req*/) const override { return TurnOn::Response{ is_on }; }

  // SetBrightness - synchronous
  SetBrightness::Response invoke(const SetBrightness::Request& r) override {
    brightness = r.level;
    return SetBrightness::Response{ brightness };
  }
  // const variant: simply return brightness (no mutation)
  SetBrightness::Response invoke(const SetBrightness::Request& /*req*/) const override { return SetBrightness::Response{ brightness }; }

  // SetBrightness - asynchronous example (simulate work)
  std::future<SetBrightness::Response> invoke_async(const SetBrightness::Request& r) override {
    return std::async(std::launch::async, [this, r]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      this->brightness = r.level;
      return SetBrightness::Response{ this->brightness };
    });
  }
  std::future<SetBrightness::Response> invoke_async(const SetBrightness::Request& /*req*/) const override {
    return std::async(std::launch::async, [this]() {
      return SetBrightness::Response{ this->brightness };
    });
  }

private:
  bool is_on     = false;
  int brightness = 0;
};