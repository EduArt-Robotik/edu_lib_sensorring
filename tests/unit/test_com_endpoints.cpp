#include <catch2/catch_all.hpp>

#include "interface/ComEndpoints.hpp"

#include <unordered_set>

using eduart::com::ComEndpoint;

TEST_CASE("ComEndpoint construction and getId", "[ComEndpoint]") {
  SECTION("construct with string id") {
    ComEndpoint ep("sensor_0");
    REQUIRE(ep.getId() == "sensor_0");
  }

  SECTION("construct with empty id") {
    ComEndpoint ep("");
    REQUIRE(ep.getId().empty());
  }

  SECTION("construct with longer id") {
    ComEndpoint ep("can://device0/endpoint/42");
    REQUIRE(ep.getId() == "can://device0/endpoint/42");
  }
}

TEST_CASE("ComEndpoint copy construction", "[ComEndpoint]") {
  ComEndpoint original("original_id");
  ComEndpoint copy(original);

  REQUIRE(copy.getId() == original.getId());
  REQUIRE(copy.getId() == "original_id");
}

TEST_CASE("ComEndpoint equality", "[ComEndpoint]") {
  ComEndpoint a("same_id");
  ComEndpoint b("same_id");
  ComEndpoint c("other_id");

  REQUIRE(a == b);
  REQUIRE(b == a);
  REQUIRE_FALSE(a == c);
  REQUIRE_FALSE(c == a);
  REQUIRE_FALSE(b == c);
}

TEST_CASE("ComEndpoint in unordered_set", "[ComEndpoint]") {
  std::unordered_set<ComEndpoint> endpoints;

  endpoints.insert(ComEndpoint("ep1"));
  endpoints.insert(ComEndpoint("ep2"));
  endpoints.insert(ComEndpoint("ep1"));  // duplicate id

  REQUIRE(endpoints.size() == 2u);

  REQUIRE(endpoints.find(ComEndpoint("ep1")) != endpoints.end());
  REQUIRE(endpoints.find(ComEndpoint("ep2")) != endpoints.end());
  REQUIRE(endpoints.find(ComEndpoint("ep3")) == endpoints.end());
}
