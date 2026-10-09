#include <catch2/catch_all.hpp>
#include <cmath>
#include <functional>
#include <limits>
#include <thread>

#include "interface/ComInterface.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"
#include "sensorring_transport/ByteOperations.hpp"
#include "sensorring_transport/Protocol.hpp"

using namespace eduart::sensorring;
namespace ws2812b = transport::protocol::ws2812b;

namespace {

class RecordingInterface : public com::ComInterface {
public:
  RecordingInterface()
      : ComInterface(com::ComInterfaceID{}) {}
  struct Message {
    com::ComEndpoint target;
    std::uint8_t command;
    std::vector<std::uint8_t> payload;
  };
  std::vector<Message> messages;
  bool send_success           = true;
  std::uint8_t failed_command = 0;
  bool reply_enabled          = true;
  bool apply_settings         = true;
  float lower_m               = 0.1F;
  float upper_m               = 0.5F;
  std::uint8_t brightness     = 50;
  std::function<void(com::ComEndpoint, std::uint8_t)> on_parameter_request;

  void reply(com::ComEndpoint target, std::uint8_t command, const std::vector<std::uint8_t>& payload) {
    target.direction = com::Direction::Output;
    dispatchMessage(target, command, payload);
  }

  std::vector<std::uint8_t> rangePayload() const {
    std::vector<std::uint8_t> payload(8);
    transport::ByteOperations::writeFloat(payload, 0, lower_m);
    transport::ByteOperations::writeFloat(payload, 4, upper_m);
    return payload;
  }

  bool send(com::ComEndpoint target, std::uint8_t command, const std::vector<std::uint8_t>& payload) override {
    messages.push_back({ target, command, payload });
    if (!send_success || command == failed_command) {
      return false;
    }
    if (command == ws2812b::PARAMETER_SET_DISTANCE_MAP_RANGE && apply_settings) {
      lower_m = transport::ByteOperations::readFloat(payload, 0);
      upper_m = transport::ByteOperations::readFloat(payload, 4);
    }
    if (command == ws2812b::PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS && apply_settings) {
      brightness = payload[0];
    }
    if (reply_enabled && (command == ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE || command == ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS)) {
      if (on_parameter_request) {
        on_parameter_request(target, command);
      } else {
        reply(target, command, command == ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE ? rangePayload() : std::vector<std::uint8_t>{ brightness });
      }
    }
    return true;
  }
  bool openInterface() override { return true; }
  bool closeInterface() override { return true; }
  bool repairInterface() override { return true; }

protected:
  bool listener() override { return true; }
};

class TestLight : public device::WS2812b_Device {
public:
  TestLight(device::WS2812b_Params params, RecordingInterface& interface, unsigned int idx = 2)
      : WS2812b_Device(std::move(params), &interface, idx) {}
};

} // namespace

TEST_CASE("Light distance map defaults preserve the original range", "[Light][Params]") {
  device::LightParams params;
  REQUIRE(params.distance_map_lower_m == 0.1);
  REQUIRE(params.distance_map_upper_m == 0.5);
  REQUIRE(params.distance_map_max_brightness == 50);
  REQUIRE_NOTHROW(params.validateDistanceMapBrightness());
  REQUIRE_NOTHROW(params.validateDistanceMapRange());
}

TEST_CASE("Light distance map rejects invalid and unrepresentable ranges", "[Light][Params]") {
  device::LightParams params;
  SECTION("negative lower") {
    params.distance_map_lower_m = -0.1;
  }
  SECTION("equal bounds") {
    params.distance_map_upper_m = 0.1;
  }
  SECTION("reversed bounds") {
    params.distance_map_upper_m = 0.05;
  }
  SECTION("NaN lower") {
    params.distance_map_lower_m = std::numeric_limits<double>::quiet_NaN();
  }
  SECTION("NaN upper") {
    params.distance_map_upper_m = std::numeric_limits<double>::quiet_NaN();
  }
  SECTION("infinite lower") {
    params.distance_map_lower_m = std::numeric_limits<double>::infinity();
  }
  SECTION("infinite upper") {
    params.distance_map_upper_m = std::numeric_limits<double>::infinity();
  }
  SECTION("float overflow") {
    params.distance_map_upper_m = std::numeric_limits<double>::max();
  }
  SECTION("bounds collapse in float32") {
    params.distance_map_upper_m = std::nextafter(0.1, 1.0);
  }
  SECTION("upper underflows to zero") {
    params.distance_map_lower_m = 0.0;
    params.distance_map_upper_m = std::numeric_limits<double>::denorm_min();
  }
  REQUIRE_THROWS_AS(params.validateDistanceMapRange(), std::invalid_argument);
}

TEST_CASE("Light distance map accepts zero and custom ranges in meters", "[Light][Params]") {
  device::LightParams params;
  params.distance_map_lower_m = 0.0;
  params.distance_map_upper_m = 1.5;
  REQUIRE_NOTHROW(params.validateDistanceMapRange());
  device::WS2812b_Params concrete(params);
  REQUIRE(concrete.distance_map_lower_m == 0.0);
  REQUIRE(concrete.distance_map_upper_m == 1.5);
}

TEST_CASE("Light configuration sends both thresholds before mode and reapplies after reset", "[Light][Configure]") {
  RecordingInterface interface;
  device::WS2812b_Params params;
  params.distance_map_lower_m        = 0.25;
  params.distance_map_upper_m        = 1.5;
  params.distance_map_max_brightness = 123;
  TestLight light(params, interface);
  light.setDeviceIndex(17);
  light.setLight(device::LightMode::MapDistance, 1, 2, 3);
  interface.messages.clear();

  for (int reset = 0; reset < 2; ++reset) {
    REQUIRE(light.configure());
    REQUIRE(interface.messages.size() == 5);
    const auto& range = interface.messages[0];
    REQUIRE(range.target == com::ComEndpoint{ com::Direction::Input, 3, transport::protocol::devbyte::WS2812B });
    REQUIRE(range.command == ws2812b::PARAMETER_SET_DISTANCE_MAP_RANGE);
    REQUIRE(range.payload == std::vector<std::uint8_t>{ 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00, 0xc0, 0x3f });
    REQUIRE(transport::ByteOperations::readFloat(range.payload, 0) == 0.25F);
    REQUIRE(transport::ByteOperations::readFloat(range.payload, 4) == 1.5F);
    REQUIRE(interface.messages[1].target == range.target);
    REQUIRE(interface.messages[1].command == ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE);
    REQUIRE(interface.messages[1].payload.empty());
    REQUIRE(interface.messages[2].command == ws2812b::PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS);
    REQUIRE(interface.messages[2].payload == std::vector<std::uint8_t>{ 123 });
    REQUIRE(interface.messages[3].target == range.target);
    REQUIRE(interface.messages[3].command == ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS);
    REQUIRE(interface.messages[3].payload.empty());
    REQUIRE(interface.messages[4].command == ws2812b::SET_LED_MODE);
    REQUIRE(interface.messages[4].payload == std::vector<std::uint8_t>{ static_cast<std::uint8_t>(device::LightMode::MapDistance), 1, 2, 3 });
    interface.lower_m    = 0.1F;
    interface.upper_m    = 0.5F;
    interface.brightness = 50;
    interface.messages.clear();
  }
}

TEST_CASE("Light configuration reports failed threshold transmission", "[Light][Configure]") {
  RecordingInterface interface;
  interface.send_success = false;
  TestLight light(device::WS2812b_Params{}, interface);
  REQUIRE_FALSE(light.configure());
  REQUIRE(interface.messages.size() == 1);
  REQUIRE(interface.messages[0].command == ws2812b::PARAMETER_SET_DISTANCE_MAP_RANGE);
}

TEST_CASE("Light construction rejects invalid thresholds before configuration", "[Light][Params]") {
  RecordingInterface interface;
  device::WS2812b_Params params;
  params.distance_map_upper_m = params.distance_map_lower_m;
  REQUIRE_THROWS_AS(TestLight(params, interface), std::invalid_argument);
  REQUIRE(interface.messages.empty());
}

TEST_CASE("Light distance map brightness accepts exact endpoints without truncation", "[Light][Params][Configure]") {
  const auto brightness = GENERATE(0, 50, 255);
  RecordingInterface interface;
  device::WS2812b_Params params;
  params.distance_map_max_brightness = brightness;
  REQUIRE_NOTHROW(params.validateDistanceMapBrightness());
  TestLight light(params, interface);
  REQUIRE(light.configure());
  REQUIRE(interface.messages.size() == 5);
  REQUIRE(interface.messages[2].command == ws2812b::PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS);
  REQUIRE(interface.messages[2].payload == std::vector<std::uint8_t>{ static_cast<std::uint8_t>(brightness) });
}

TEST_CASE("Light distance map brightness rejects out-of-range values", "[Light][Params]") {
  const auto brightness = GENERATE(-1LL, 256LL, std::numeric_limits<long long>::max());
  RecordingInterface interface;
  device::WS2812b_Params params;
  params.distance_map_max_brightness = brightness;
  REQUIRE_THROWS_AS(params.validateDistanceMapBrightness(), std::invalid_argument);
  REQUIRE_THROWS_AS(TestLight(params, interface), std::invalid_argument);
  REQUIRE(interface.messages.empty());
}

TEST_CASE("Light configuration reports failed brightness transmission before mode restoration", "[Light][Configure]") {
  RecordingInterface interface;
  interface.failed_command = ws2812b::PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS;
  TestLight light(device::WS2812b_Params{}, interface);
  REQUIRE_FALSE(light.configure());
  REQUIRE(interface.messages.size() == 3);
  REQUIRE(interface.messages[2].command == ws2812b::PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS);
}

TEST_CASE("Light getters read live board parameters in SI units", "[Light][Getters]") {
  RecordingInterface interface;
  interface.lower_m    = 0.3F;
  interface.upper_m    = 2.5F;
  interface.brightness = 200;
  TestLight light(device::WS2812b_Params{}, interface);
  double lower_m          = 0.0;
  double upper_m          = 0.0;
  std::int64_t brightness = 0;
  REQUIRE(light.getDistanceMapRange(lower_m, upper_m));
  REQUIRE(lower_m == static_cast<double>(interface.lower_m));
  REQUIRE(upper_m == 2.5);
  REQUIRE(light.getDistanceMapMaxBrightness(brightness));
  REQUIRE(brightness == 200);
  REQUIRE(interface.messages.size() == 2);
  REQUIRE(interface.messages[0].command == (ws2812b::PARAMETER_SET_DISTANCE_MAP_RANGE | transport::protocol::PARAMETER_GETTER_BYTE));
  REQUIRE(interface.messages[1].command == (ws2812b::PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS | transport::protocol::PARAMETER_GETTER_BYTE));
}

TEST_CASE("Light setters accept float32 rounding but reject incorrect readback", "[Light][Configure]") {
  RecordingInterface interface;
  TestLight light(device::WS2812b_Params{}, interface);
  bool expected_success = true;
  double lower_m        = 0.123456789;
  double upper_m        = 1.987654321;
  SECTION("ordinary float32 conversion") {
  }
  SECTION("small rounding difference") {
    interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
      interface.lower_m += 5.0e-7F;
      interface.upper_m += 5.0e-7F;
      interface.reply(target, command, interface.rangePayload());
    };
  }
  SECTION("lower beyond tolerance") {
    expected_success               = false;
    interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
      interface.lower_m += 2.0e-6F;
      interface.reply(target, command, interface.rangePayload());
    };
  }
  SECTION("upper beyond relative tolerance") {
    expected_success               = false;
    interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
      interface.upper_m += 4.0e-6F;
      interface.reply(target, command, interface.rangePayload());
    };
  }
  REQUIRE(light.setDistanceMapRange(lower_m, upper_m) == expected_success);
  REQUIRE(light.getParams().distance_map_lower_m == (expected_success ? lower_m : 0.1));
  REQUIRE(light.getParams().distance_map_upper_m == (expected_success ? upper_m : 0.5));
}

TEST_CASE("Light brightness verification requires an exact match", "[Light][Configure]") {
  RecordingInterface interface;
  interface.apply_settings = false;
  TestLight light(device::WS2812b_Params{}, interface);
  REQUIRE_FALSE(light.setDistanceMapMaxBrightness(51));
  REQUIRE(light.getParams().distance_map_max_brightness == 50);
  interface.apply_settings = true;
  REQUIRE(light.setDistanceMapMaxBrightness(51));
  REQUIRE(light.getParams().distance_map_max_brightness == 51);
}

TEST_CASE("Light configuration fails when firmware ignores parameter setters", "[Light][Configure]") {
  RecordingInterface interface;
  interface.apply_settings = false;
  device::WS2812b_Params params;
  SECTION("range mismatch") {
    params.distance_map_lower_m = 0.25;
  }
  SECTION("brightness mismatch") {
    params.distance_map_max_brightness = 123;
  }
  TestLight light(params, interface);
  REQUIRE_FALSE(light.configure());
  REQUIRE(interface.messages.back().command != ws2812b::SET_LED_MODE);
}

TEST_CASE("Light getters preserve output values on timeout and failed transmission", "[Light][Getters]") {
  RecordingInterface interface;
  SECTION("timeout or older firmware") {
    interface.reply_enabled = false;
  }
  SECTION("failed range getter send") {
    interface.failed_command = ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE;
  }
  SECTION("failed brightness getter send") {
    interface.failed_command = ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS;
  }
  TestLight light(device::WS2812b_Params{}, interface);
  double lower_m          = 7.0;
  double upper_m          = 8.0;
  std::int64_t brightness = 999;
  if (!interface.reply_enabled || interface.failed_command == ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE) {
    REQUIRE_FALSE(light.getDistanceMapRange(lower_m, upper_m));
    REQUIRE(lower_m == 7.0);
    REQUIRE(upper_m == 8.0);
  }
  if (!interface.reply_enabled || interface.failed_command == ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS) {
    REQUIRE_FALSE(light.getDistanceMapMaxBrightness(brightness));
    REQUIRE(brightness == 999);
  }
}

TEST_CASE("Light configuration fails when getter verification cannot complete", "[Light][Configure]") {
  RecordingInterface interface;
  SECTION("missing replies") {
    interface.reply_enabled = false;
  }
  SECTION("failed range getter send") {
    interface.failed_command = ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE;
  }
  SECTION("failed brightness getter send") {
    interface.failed_command = ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS;
  }
  TestLight light(device::WS2812b_Params{}, interface);
  REQUIRE_FALSE(light.configure());
  REQUIRE(interface.messages.back().command != ws2812b::SET_LED_MODE);
}

TEST_CASE("Light getters reject malformed and invalid range replies", "[Light][Getters]") {
  RecordingInterface interface;
  auto payload = interface.rangePayload();
  SECTION("short payload") {
    payload.resize(7);
  }
  SECTION("oversized payload") {
    payload.push_back(0);
  }
  SECTION("NaN") {
    transport::ByteOperations::writeFloat(payload, 0, std::numeric_limits<float>::quiet_NaN());
  }
  SECTION("infinity") {
    transport::ByteOperations::writeFloat(payload, 4, std::numeric_limits<float>::infinity());
  }
  SECTION("negative lower") {
    transport::ByteOperations::writeFloat(payload, 0, -0.1F);
  }
  SECTION("equal bounds") {
    transport::ByteOperations::writeFloat(payload, 4, 0.1F);
  }
  SECTION("reversed bounds") {
    transport::ByteOperations::writeFloat(payload, 4, 0.05F);
  }
  interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
    interface.reply(target, command, payload);
  };
  TestLight light(device::WS2812b_Params{}, interface);
  double lower_m = 7.0;
  double upper_m = 8.0;
  REQUIRE_FALSE(light.getDistanceMapRange(lower_m, upper_m));
  REQUIRE(lower_m == 7.0);
  REQUIRE(upper_m == 8.0);
}

TEST_CASE("Light getters reject malformed brightness replies", "[Light][Getters]") {
  RecordingInterface interface;
  const auto payload             = GENERATE(std::vector<std::uint8_t>{}, std::vector<std::uint8_t>{ 50, 0 });
  interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
    interface.reply(target, command, payload);
  };
  TestLight light(device::WS2812b_Params{}, interface);
  std::int64_t brightness = 999;
  REQUIRE_FALSE(light.getDistanceMapMaxBrightness(brightness));
  REQUIRE(brightness == 999);
}

TEST_CASE("Light getters wait for the matching command and board response", "[Light][Getters]") {
  RecordingInterface interface;
  interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
    interface.reply(target, ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS, { 200 });
    auto other_board         = target;
    other_board.boardAddress = 4;
    interface.lower_m        = 0.2F;
    interface.upper_m        = 0.8F;
    interface.reply(other_board, command, interface.rangePayload());
    auto other_device     = target;
    other_device.deviceId = transport::protocol::devbyte::TMF8829;
    interface.reply(other_device, command, interface.rangePayload());
    interface.lower_m = 0.1F;
    interface.upper_m = 0.5F;
    interface.reply(target, command, interface.rangePayload());
  };
  TestLight light(device::WS2812b_Params{}, interface);
  double lower_m = 0.0;
  double upper_m = 0.0;
  REQUIRE(light.getDistanceMapRange(lower_m, upper_m));
  REQUIRE(lower_m == static_cast<double>(interface.lower_m));
  REQUIRE(upper_m == static_cast<double>(interface.upper_m));
}

TEST_CASE("Light getters time out when only unrelated replies arrive", "[Light][Getters]") {
  RecordingInterface interface;
  bool wrong_command = false;
  bool wrong_board   = false;
  bool wrong_device  = false;
  SECTION("wrong command") {
    wrong_command = true;
  }
  SECTION("wrong board") {
    wrong_board = true;
  }
  SECTION("wrong device") {
    wrong_device = true;
  }
  interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
    if (wrong_command) {
      command = ws2812b::PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS;
    }
    if (wrong_board) {
      target.boardAddress = 4;
    }
    if (wrong_device) {
      target.deviceId = transport::protocol::devbyte::TMF8829;
    }
    interface.reply(target, command, command == ws2812b::PARAMETER_GET_DISTANCE_MAP_RANGE ? interface.rangePayload() : std::vector<std::uint8_t>{ 50 });
  };
  TestLight light(device::WS2812b_Params{}, interface);
  double lower_m = 7.0;
  double upper_m = 8.0;
  REQUIRE_FALSE(light.getDistanceMapRange(lower_m, upper_m));
  REQUIRE(lower_m == 7.0);
  REQUIRE(upper_m == 8.0);
}

TEST_CASE("Light getters receive asynchronous listener responses", "[Light][Getters]") {
  RecordingInterface interface;
  std::thread listener;
  interface.on_parameter_request = [&](com::ComEndpoint target, std::uint8_t command) {
    listener = std::thread([&, target, command]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      interface.reply(target, command, { 123 });
    });
  };
  TestLight light(device::WS2812b_Params{}, interface);
  std::int64_t brightness = 0;
  const bool success      = light.getDistanceMapMaxBrightness(brightness);
  listener.join();
  REQUIRE(success);
  REQUIRE(brightness == 123);
}

TEST_CASE("Light public setters reject invalid parameters without transmission", "[Light][Params]") {
  RecordingInterface interface;
  TestLight light(device::WS2812b_Params{}, interface);
  REQUIRE_FALSE(light.setDistanceMapRange(-0.1, 0.5));
  REQUIRE_FALSE(light.setDistanceMapRange(0.5, 0.5));
  REQUIRE_FALSE(light.setDistanceMapMaxBrightness(-1));
  REQUIRE_FALSE(light.setDistanceMapMaxBrightness(256));
  REQUIRE(interface.messages.empty());
}
