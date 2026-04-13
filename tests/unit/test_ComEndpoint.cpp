#include <catch2/catch_all.hpp>
#include <sensorring_transport/Protocol.hpp>
#include <unordered_map>
#include <unordered_set>
using namespace eduart::sensorring::transport::protocol;
#include "sensorring/interface/ComEndpoint.hpp"

using eduart::sensorring::com::ComEndpoint;
using eduart::sensorring::com::Direction;
using eduart::sensorring::com::endpointMatches;

TEST_CASE("ComEndpoint equality", "[ComEndpoint]") {
  ComEndpoint a{ Direction::Output, 6, devbyte::VL53L8CX };
  ComEndpoint b{ Direction::Output, 6, devbyte::VL53L8CX };
  ComEndpoint c{ Direction::Output, 6, devbyte::HTPA32 };

  REQUIRE(a == b);
  REQUIRE(a != c);
}

TEST_CASE("ComEndpoint hashing", "[ComEndpoint]") {
  ComEndpoint a{ Direction::Output, 1, devbyte::VL53L8CX };
  ComEndpoint b{ Direction::Output, 1, devbyte::VL53L8CX };
  ComEndpoint c{ Direction::Input, 1, devbyte::VL53L8CX };

  REQUIRE(std::hash<ComEndpoint>{}(a) == std::hash<ComEndpoint>{}(b));
  REQUIRE(std::hash<ComEndpoint>{}(a) != std::hash<ComEndpoint>{}(c));
}

TEST_CASE("ComEndpoint in unordered containers", "[ComEndpoint]") {
  std::unordered_set<ComEndpoint> s;
  s.insert({ Direction::Output, 1, devbyte::VL53L8CX });
  s.insert({ Direction::Output, 1, devbyte::VL53L8CX }); // duplicate
  s.insert({ Direction::Output, 2, devbyte::VL53L8CX });
  REQUIRE(s.size() == 2);

  std::unordered_map<ComEndpoint, int> m;
  m[{ Direction::Output, 5, devbyte::BOARD }] = 42;
  REQUIRE(m[{ Direction::Output, 5, devbyte::BOARD }] == 42);
}

TEST_CASE("ComEndpoint toString", "[ComEndpoint]") {
  REQUIRE(ComEndpoint{ Direction::Output, 6, devbyte::VL53L8CX }.toString() == "Output/Board5/VL53L8CX");
  REQUIRE(ComEndpoint{ Direction::Broadcast, ComEndpoint::BROADCAST, devbyte::BOARD }.toString() == "Broadcast/All/Board");
  REQUIRE(ComEndpoint{ Direction::Output, ComEndpoint::ANY_BOARD, devbyte::BOARD }.toString() == "Output/ANY/Board");
  REQUIRE(ComEndpoint{ Direction::Output, 1, devbyte::WS2812B }.toString() == "Output/Board0/WS2812b");
  REQUIRE(ComEndpoint{ Direction::Input, 1, 0x10 }.toString() == "Input/Board0/Dev16");
}

TEST_CASE("ComEndpoint endpointMatches", "[ComEndpoint]") {
  ComEndpoint sub{ Direction::Output, ComEndpoint::ANY_BOARD, devbyte::VL53L8CX };
  REQUIRE(endpointMatches(sub, { Direction::Output, 5, devbyte::VL53L8CX }));
  REQUIRE(endpointMatches(sub, { Direction::Output, 100, devbyte::VL53L8CX }));
  REQUIRE_FALSE(endpointMatches(sub, { Direction::Input, 5, devbyte::VL53L8CX }));
  REQUIRE_FALSE(endpointMatches(sub, { Direction::Output, 5, devbyte::HTPA32 }));

  ComEndpoint exact{ Direction::Output, 6, devbyte::VL53L8CX };
  REQUIRE(endpointMatches(exact, { Direction::Output, 6, devbyte::VL53L8CX }));
  REQUIRE_FALSE(endpointMatches(exact, { Direction::Output, 7, devbyte::VL53L8CX }));
}

TEST_CASE("ComEndpoint constants", "[ComEndpoint]") {
  REQUIRE(ComEndpoint::BROADCAST == 0x00);
  REQUIRE(ComEndpoint::ANY_BOARD == 0xFF);
}
