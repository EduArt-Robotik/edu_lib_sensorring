#include <catch2/catch_all.hpp>

#include "sensorring/measurement/TofMeasurement.hpp"

using eduart::sensorring::measurement::TofMeasurement;

TEST_CASE("TofMeasurement default initialization", "[TofMeasurement]") {
  TofMeasurement m;

  REQUIRE(m.frame_id == 0u);
  REQUIRE(m.point_cloud.data.empty());
}

TEST_CASE("TofMeasurement structure integrity", "[TofMeasurement]") {
  TofMeasurement m;
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
