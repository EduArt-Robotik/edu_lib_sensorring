#include <catch2/catch_all.hpp>

#include "sensorring/measurement/DepthMeasurement.hpp"

using eduart::sensorring::measurement::DepthMeasurement;

TEST_CASE("DepthMeasurement default initialization", "[DepthMeasurement]") {
  DepthMeasurement m;

  REQUIRE(m.frame_id == 0u);
  REQUIRE(m.point_cloud.data.empty());
}

TEST_CASE("DepthMeasurement structure integrity", "[DepthMeasurement]") {
  DepthMeasurement m;
  m.frame_id = 42u;

  m.point_cloud.data.resize(1);
  m.point_cloud.data[0].point.x() = 1.0;
  m.point_cloud.data[0].point.y() = 2.0;
  m.point_cloud.data[0].point.z() = 3.0;

  REQUIRE(m.frame_id == 42u);
  REQUIRE(m.point_cloud.data.size() == 1u);
  REQUIRE(m.point_cloud.data[0].point.x() == Catch::Approx(1.0));
  REQUIRE(m.point_cloud.data[0].point.y() == Catch::Approx(2.0));
  REQUIRE(m.point_cloud.data[0].point.z() == Catch::Approx(3.0));
}
