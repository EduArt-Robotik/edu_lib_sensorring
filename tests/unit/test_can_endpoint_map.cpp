#include <catch2/catch_all.hpp>
#include <cstdint>

#include "interface/can/CanEndpointMap.hpp"
#include "interface/can/canprotocol.hpp"
#include "sensorring/interface/ComEndpoint.hpp"

using eduart::com::CanEndpointMap;
using eduart::com::CanProtocol;
using eduart::com::ComEndpoint;

TEST_CASE("CanEndpointMap getInstance returns singleton", "[CanEndpointMap]") {
  CanEndpointMap* a = CanEndpointMap::getInstance();
  CanEndpointMap* b = CanEndpointMap::getInstance();

  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(a == b);
}

TEST_CASE("CanEndpointMap broadcast round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  ComEndpoint broadcast("broadcast");
  CanProtocol::canid id  = map->mapEndpointToId(broadcast);
  ComEndpoint round_trip = map->mapIdToEndpoint(id);

  REQUIRE(round_trip.getId() == broadcast.getId());
}

TEST_CASE("CanEndpointMap ToF endpoints round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("tof0_data"))).getId() == "tof0_data");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("tof_status"))).getId() == "tof_status");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("tof_request"))).getId() == "tof_request");
}

TEST_CASE("CanEndpointMap Thermal endpoints round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("thermal0_data"))).getId() == "thermal0_data");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("thermal_status"))).getId() == "thermal_status");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("thermal_request"))).getId() == "thermal_request");
}

TEST_CASE("CanEndpointMap Light endpoint round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("light"))).getId() == "light");
}

TEST_CASE("CanEndpointMap reserves indices for many sensor boards", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  // Last reserved index is MAX_SENSOR_BOARDS - 1
  std::string tof_last = "tof" + std::to_string(CanEndpointMap::MAX_SENSOR_BOARDS - 1) + "_data";
  std::string thermal_last = "thermal" + std::to_string(CanEndpointMap::MAX_SENSOR_BOARDS - 1) + "_data";

  REQUIRE_NOTHROW(map->mapEndpointToId(ComEndpoint(tof_last)));
  REQUIRE_NOTHROW(map->mapEndpointToId(ComEndpoint(thermal_last)));
}

TEST_CASE("CanEndpointMap mapEndpointToId throws for unknown endpoint", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  REQUIRE_THROWS_AS(map->mapEndpointToId(ComEndpoint("__nonexistent_endpoint__")), std::out_of_range);
}

TEST_CASE("CanEndpointMap mapIdToEndpoint throws for unknown CAN ID", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  constexpr std::uint32_t unknown_id = 0xDEADBEEF;
  REQUIRE_THROWS_AS(map->mapIdToEndpoint(unknown_id), std::runtime_error);
  REQUIRE_THROWS_WITH(map->mapIdToEndpoint(unknown_id), "No Endpoint found for given CAN ID");
}
