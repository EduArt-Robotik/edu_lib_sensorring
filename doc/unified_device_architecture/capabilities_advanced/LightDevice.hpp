#pragma once

#include <chrono>
#include <string>
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

// New: firmware/version capability implemented as a static function
struct GetFirmware {
  struct Request {};
  struct Response {
    std::string version;
  };
};

// Light device: implements TurnOn (sync) and SetBrightness (sync + async).
// Also registers a static (free-like) capability GetFirmware via register_function.
struct LightDevice : IDevice, ICapability<TurnOn>, ICapability<SetBrightness>, ICapabilityAsync<SetBrightness> {
  LightDevice() {
    // explicit registration calls (typed invokers created for each Cap)
    register_capability<TurnOn>("turn_on");
    register_capability<SetBrightness>("set_brightness");
    register_capability_async<SetBrightness>("set_brightness_async"); // async optional

    // register a static function for firmware/version capability
    // This demonstrates registering a function instead of implementing an instance method.
    register_function<GetFirmware>(&LightDevice::static_get_firmware, "firmware_version");
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

  // Static function used as a capability target (no need for per-instance state)
  static GetFirmware::Response static_get_firmware(const GetFirmware::Request&) {
    // In real code this could query a compile-time constant, embedded resource, or call platform API.
    return GetFirmware::Response{ "LightDeviceFW v1.2.3" };
  }

private:
  bool is_on     = false;
  int brightness = 0;
};