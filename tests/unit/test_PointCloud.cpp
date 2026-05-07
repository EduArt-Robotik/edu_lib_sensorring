#include <catch2/catch_all.hpp>

#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/PointCloud.hpp"

using eduart::sensorring::math::Matrix3;
using eduart::sensorring::math::rotMatrixFromEulerDegrees;
using eduart::sensorring::math::Vector3;
using eduart::sensorring::measurement::PointCloud;
using eduart::sensorring::measurement::PointData;

TEST_CASE("PointCloud PointData structure", "[PointCloud]") {
  PointData p;
  p.point.x()    = 1.0;
  p.point.y()    = 2.0;
  p.point.z()    = 3.0;
  p.raw_distance = 4.5;
  p.sigma        = 0.1;
  p.sensor_index = 7;

  REQUIRE(p.point.x() == Catch::Approx(1.0));
  REQUIRE(p.point.y() == Catch::Approx(2.0));
  REQUIRE(p.point.z() == Catch::Approx(3.0));
  REQUIRE(p.raw_distance == Catch::Approx(4.5));
  REQUIRE(p.sigma == Catch::Approx(0.1));
  REQUIRE(p.sensor_index == 7);
}

TEST_CASE("PointCloud copyTo basic behavior", "[PointCloud]") {
  PointCloud cloud;
  cloud.data.resize(2);

  cloud.data[0].point.x()    = 1.0;
  cloud.data[0].point.y()    = 2.0;
  cloud.data[0].point.z()    = 3.0;
  cloud.data[0].raw_distance = 4.0;
  cloud.data[0].sigma        = 0.1;
  cloud.data[0].sensor_index = 1;

  cloud.data[1].point.x()    = -1.0;
  cloud.data[1].point.y()    = -2.0;
  cloud.data[1].point.z()    = -3.0;
  cloud.data[1].raw_distance = 5.0;
  cloud.data[1].sigma        = 0.2;
  cloud.data[1].sensor_index = 2;

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
      cloud.data[i].sensor_index = i;
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

TEST_CASE("PointCloud transform applies rotation and translation", "[PointCloud]") {
  PointCloud cloud;
  cloud.data.resize(2);

  cloud.data[0].point.x() = 1.0;
  cloud.data[0].point.y() = 0.0;
  cloud.data[0].point.z() = 0.0;

  cloud.data[1].point.x() = 0.0;
  cloud.data[1].point.y() = 1.0;
  cloud.data[1].point.z() = 0.0;

  Vector3 euler_deg{};
  euler_deg.z()     = 90.0;
  const Matrix3 rot = rotMatrixFromEulerDegrees(euler_deg);

  Vector3 translation{};
  translation.x() = 1.0;
  translation.y() = 2.0;
  translation.z() = 3.0;

  const PointCloud transformed = PointCloud::transform(cloud, rot, translation);

  REQUIRE(transformed.data[0].point.x() == Catch::Approx(1.0));
  REQUIRE(transformed.data[0].point.y() == Catch::Approx(3.0));
  REQUIRE(transformed.data[0].point.z() == Catch::Approx(3.0));

  REQUIRE(transformed.data[1].point.x() == Catch::Approx(0.0));
  REQUIRE(transformed.data[1].point.y() == Catch::Approx(2.0));
  REQUIRE(transformed.data[1].point.z() == Catch::Approx(3.0));

  Vector3 inverse_euler{};
  inverse_euler.z()             = -90.0;
  const Matrix3 inv_rot         = rotMatrixFromEulerDegrees(inverse_euler);
  const Vector3 inv_translation = (inv_rot * translation) * -1.0;

  const PointCloud roundtrip = PointCloud::transform(transformed, inv_rot, inv_translation);

  REQUIRE(roundtrip.data[0].point.x() == Catch::Approx(cloud.data[0].point.x()).margin(1e-6));
  REQUIRE(roundtrip.data[0].point.y() == Catch::Approx(cloud.data[0].point.y()).margin(1e-6));
  REQUIRE(roundtrip.data[0].point.z() == Catch::Approx(cloud.data[0].point.z()).margin(1e-6));

  REQUIRE(roundtrip.data[1].point.x() == Catch::Approx(cloud.data[1].point.x()).margin(1e-6));
  REQUIRE(roundtrip.data[1].point.y() == Catch::Approx(cloud.data[1].point.y()).margin(1e-6));
  REQUIRE(roundtrip.data[1].point.z() == Catch::Approx(cloud.data[1].point.z()).margin(1e-6));
}
