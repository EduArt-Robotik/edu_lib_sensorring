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
// Also registers a static (device-scoped) capability GetFirmware via register_static_function_for.
struct LightDevice : IDevice, ICapability<TurnOn>, ICapability<SetBrightness>, ICapabilityAsync<SetBrightness> {
  LightDevice() {
    // instance registration
    register_capability<TurnOn>("turn_on");
    register_capability<SetBrightness>("set_brightness");
    register_capability_async<SetBrightness>("set_brightness_async");

    // Note: Static function registration for GetFirmware is done at namespace scope
    // (see below) so it's available before any instance is created.
    // You may also register an async static function with register_static_function_async
  }

  // TurnOn (mutating)
  TurnOn::Response invoke(const TurnOn::Request&) override {
    is_on = true;
    return TurnOn::Response{ true };
  }
  TurnOn::Response invoke(const TurnOn::Request& /*req*/) const override { return TurnOn::Response{ is_on }; }

  // SetBrightness - synchronous
  SetBrightness::Response invoke(const SetBrightness::Request& r) override {
    brightness = r.level;
    return SetBrightness::Response{ brightness };
  }
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

  // Static function used as a globally-registered capability target for LightDevice only
  static GetFirmware::Response static_get_firmware(const GetFirmware::Request&) { return GetFirmware::Response{ "LightDeviceFW v1.2.3" }; }

private:
  bool is_on     = false;
  int brightness = 0;
};

// Register the static function at namespace scope so it's available before any instance is created.
// This ensures that IDevice::invoke_static<GetFirmware>() can be called without creating a LightDevice instance.
namespace {
// This static variable initializer runs before main(), ensuring the static function is registered
// as soon as this translation unit is loaded.
static bool _light_device_static_init = []() {
  IDevice::register_static_function_for<LightDevice, GetFirmware>(&LightDevice::static_get_firmware);
  return true;
}();
} // namespace