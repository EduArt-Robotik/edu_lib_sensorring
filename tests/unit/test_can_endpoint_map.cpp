#include <catch2/catch_all.hpp>

#include <cstdint>

#include "interface/ComEndpoints.hpp"
#include "interface/can/CanEndpointMap.hpp"
#include "interface/can/canprotocol.hpp"

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

TEST_CASE("CanEndpointMap addSensorBoardEndpoint and round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  map->addSensorBoardEndpoint();

  ComEndpoint broadcast("broadcast");
  CanProtocol::canid id = map->mapEndpointToId(broadcast);
  ComEndpoint round_trip = map->mapIdToEndpoint(id);

  REQUIRE(round_trip.getId() == broadcast.getId());
}

TEST_CASE("CanEndpointMap addTofSensorEndpoint and round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  map->addTofSensorEndpoint(0);

  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("tof0_data"))).getId() == "tof0_data");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("tof_status"))).getId() == "tof_status");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("tof_request"))).getId() == "tof_request");
}

TEST_CASE("CanEndpointMap addThermalSensorEndpoint and round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  map->addThermalSensorEndpoint(0);

  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("thermal0_data"))).getId() == "thermal0_data");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("thermal_status"))).getId() == "thermal_status");
  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("thermal_request"))).getId() == "thermal_request");
}

TEST_CASE("CanEndpointMap addLightSensorEndpoint and round-trip", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  map->addLightSensorEndpoint();

  REQUIRE(map->mapIdToEndpoint(map->mapEndpointToId(ComEndpoint("light"))).getId() == "light");
}

TEST_CASE("CanEndpointMap mapEndpointToId throws for unknown endpoint", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  REQUIRE_THROWS_AS(map->mapEndpointToId(ComEndpoint("__nonexistent_endpoint__")), std::out_of_range);
}

TEST_CASE("CanEndpointMap mapIdToEndpoint throws for unknown CAN ID", "[CanEndpointMap]") {
  CanEndpointMap* map = CanEndpointMap::getInstance();

  // Use an ID that is not produced by the protocol (0xDEADBEEF)
  constexpr std::uint32_t unknown_id = 0xDEADBEEF;
  REQUIRE_THROWS_AS(map->mapIdToEndpoint(unknown_id), std::runtime_error);
  REQUIRE_THROWS_WITH(map->mapIdToEndpoint(unknown_id), "No Endpoint found for given CAN ID");
}
