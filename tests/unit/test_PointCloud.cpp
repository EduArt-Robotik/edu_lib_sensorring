#include <catch2/catch_all.hpp>

#include "sensorring/measurement/PointCloud.hpp"

using eduart::measurement::PointCloud;
using eduart::measurement::PointData;

TEST_CASE("PointCloud PointData structure", "[PointCloud]") {
  PointData p;
  p.point.x()    = 1.0;
  p.point.y()    = 2.0;
  p.point.z()    = 3.0;
  p.raw_distance = 4.5;
  p.sigma        = 0.1;
  p.user_idx     = 7;

  REQUIRE(p.point.x() == Catch::Approx(1.0));
  REQUIRE(p.point.y() == Catch::Approx(2.0));
  REQUIRE(p.point.z() == Catch::Approx(3.0));
  REQUIRE(p.raw_distance == Catch::Approx(4.5));
  REQUIRE(p.sigma == Catch::Approx(0.1));
  REQUIRE(p.user_idx == 7);
}

TEST_CASE("PointCloud copyTo basic behavior", "[PointCloud]") {
  PointCloud cloud;
  cloud.data.resize(2);

  cloud.data[0].point.x()    = 1.0;
  cloud.data[0].point.y()    = 2.0;
  cloud.data[0].point.z()    = 3.0;
  cloud.data[0].raw_distance = 4.0;
  cloud.data[0].sigma        = 0.1;
  cloud.data[0].user_idx     = 1;

  cloud.data[1].point.x()    = -1.0;
  cloud.data[1].point.y()    = -2.0;
  cloud.data[1].point.z()    = -3.0;
  cloud.data[1].raw_distance = 5.0;
  cloud.data[1].sigma        = 0.2;
  cloud.data[1].user_idx     = 2;

  double buffer[12] = { 0.0 };
  cloud.copyTo(buffer, 12);

  REQUIRE(buffer[0] == Catch::Approx(1.0));
  REQUIRE(buffer[1] == Catch::Approx(2.0));
  REQUIRE(buffer[2] == Catch::Approx(3.0));
  REQUIRE(buffer[3] == Catch::Approx(4.0));
  REQUIRE(buffer[4] == Catch::Approx(0.1));
  REQUIRE(buffer[5] == Catch::Approx(1.0));

  REQUIRE(buffer[6] == Catch::Approx(-1.0));
  REQUIRE(buffer[7] == Catch::Approx(-2.0));
  REQUIRE(buffer[8] == Catch::Approx(-3.0));
  REQUIRE(buffer[9] == Catch::Approx(5.0));
  REQUIRE(buffer[10] == Catch::Approx(0.2));
  REQUIRE(buffer[11] == Catch::Approx(2.0));
}

TEST_CASE("PointCloud copyTo buffer clipping and empty cloud", "[PointCloud]") {
  SECTION("Empty cloud writes nothing") {
    PointCloud cloud;
    double buffer[6] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    cloud.copyTo(buffer, 6);

    // Nothing should be overwritten because there are no points
    REQUIRE(buffer[0] == Catch::Approx(1.0));
    REQUIRE(buffer[5] == Catch::Approx(6.0));
  }

  SECTION("Buffer smaller than number of points") {
    PointCloud cloud;
    cloud.data.resize(3);
    for (int i = 0; i < 3; ++i) {
      cloud.data[i].point.x()    = i + 1.0;
      cloud.data[i].point.y()    = (i + 1.0) * 10.0;
      cloud.data[i].point.z()    = (i + 1.0) * 100.0;
      cloud.data[i].raw_distance = i + 0.5;
      cloud.data[i].sigma        = 0.01 * (i + 1);
      cloud.data[i].user_idx     = i;
    }

    // Buffer can only hold one point (6 doubles)
    double buffer[6] = { 0.0 };
    cloud.copyTo(buffer, 6);

    REQUIRE(buffer[0] == Catch::Approx(1.0));
    REQUIRE(buffer[1] == Catch::Approx(10.0));
    REQUIRE(buffer[2] == Catch::Approx(100.0));
    REQUIRE(buffer[3] == Catch::Approx(0.5));
    REQUIRE(buffer[4] == Catch::Approx(0.01));
    REQUIRE(buffer[5] == Catch::Approx(0.0));
  }
}
