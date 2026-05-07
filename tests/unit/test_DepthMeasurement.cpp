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

TEST_CASE("DepthMeasurement::combinePointClouds uses transformed clouds", "[DepthMeasurement]") {
  DepthMeasurement m0, m1;
  m0.point_cloud.data.push_back({ { 1.0, 0.0, 0.0 }, 1.0, 0.01, 0 });
  m0.transformed_point_cloud.data.push_back({ { 10.0, 0.0, 0.0 }, 1.0, 0.01, 0 });
  m1.point_cloud.data.push_back({ { 2.0, 0.0, 0.0 }, 2.0, 0.02, 1 });
  m1.transformed_point_cloud.data.push_back({ { 20.0, 0.0, 0.0 }, 2.0, 0.02, 1 });

  auto combined = DepthMeasurement::combinePointClouds({ m0, m1 });

  REQUIRE(combined.data.size() == 2);
  REQUIRE(combined.data[0].point.x() == Catch::Approx(10.0));
  REQUIRE(combined.data[1].point.x() == Catch::Approx(20.0));
}

TEST_CASE("DepthMeasurement::combinePointClouds with empty vector", "[DepthMeasurement]") {
  auto combined = DepthMeasurement::combinePointClouds({});
  REQUIRE(combined.data.empty());
}
