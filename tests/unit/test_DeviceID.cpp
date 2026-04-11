// Unit tests for DeviceID (device type, name, index) and DeviceType enum.

#include <catch2/catch_all.hpp>

#include "sensorring/device/DeviceID.hpp"

using eduart::sensorring::device::DeviceID;
using eduart::sensorring::device::DeviceType;

TEST_CASE("DeviceID construction and defaults", "[DeviceID]") {
  SECTION("default construction") {
    DeviceID id{};
    REQUIRE(id.type == DeviceType::UNDEFINED);
    REQUIRE(id.index == 0u);
  }

  SECTION("aggregate initialization") {
    DeviceID id{ DeviceType::VL53L8CX, 1u };
    REQUIRE(id.type == DeviceType::VL53L8CX);
    REQUIRE(id.index == 1u);
  }

  SECTION("all DeviceType values") {
    DeviceID vl53{ DeviceType::VL53L8CX, 0u };
    DeviceID htpa{ DeviceType::HTPA32, 0u };
    DeviceID ws2812{ DeviceType::WS2812b, 0u };
    DeviceID undef{ DeviceType::UNDEFINED, 0u };
    REQUIRE(vl53.type == DeviceType::VL53L8CX);
    REQUIRE(htpa.type == DeviceType::HTPA32);
    REQUIRE(ws2812.type == DeviceType::WS2812b);
    REQUIRE(undef.type == DeviceType::UNDEFINED);
  }
}

TEST_CASE("DeviceID getters", "[DeviceID]") {
  SECTION("getType") {
    DeviceID id{ DeviceType::HTPA32, 2u };
    REQUIRE(id.getType() == DeviceType::HTPA32);
  }

  SECTION("getIndex") {
    DeviceID id{ DeviceType::WS2812b, 3u };
    REQUIRE(id.getIndex() == 3u);
  }

  SECTION("getters reflect direct member changes") {
    DeviceID id{ DeviceType::VL53L8CX, 0u };
    id.type  = DeviceType::HTPA32;
    id.index = 5u;
    REQUIRE(id.getType() == DeviceType::HTPA32);
    REQUIRE(id.getIndex() == 5u);
  }
}

TEST_CASE("DeviceID isValid", "[DeviceID]") {
  SECTION("default constructed is invalid") {
    DeviceID id{};
    REQUIRE_FALSE(id.isValid());
  }

  SECTION("UNDEFINED type is invalid even with name") {
    DeviceID id{ DeviceType::UNDEFINED, 0u };
    REQUIRE_FALSE(id.isValid());
  }

  SECTION("valid when type is not UNDEFINED and name is non-empty") {
    DeviceID id{ DeviceType::VL53L8CX, 0u };
    REQUIRE(id.isValid());
  }

  SECTION("valid for all defined device types with non-empty name") {
    REQUIRE(DeviceID{ DeviceType::VL53L8CX, 0u }.isValid());
    REQUIRE(DeviceID{ DeviceType::HTPA32, 0u }.isValid());
    REQUIRE(DeviceID{ DeviceType::WS2812b, 0u }.isValid());
  }

  SECTION("index does not affect validity") {
    DeviceID id{ DeviceType::VL53L8CX, 99u };
    REQUIRE(id.isValid());
    REQUIRE(id.getIndex() == 99u);
  }
}

TEST_CASE("DeviceID edge cases", "[DeviceID]") {
  SECTION("index zero") {
    DeviceID id{ DeviceType::VL53L8CX, 0u };
    REQUIRE(id.getIndex() == 0u);
    REQUIRE(id.isValid());
  }
}
