#include <catch2/catch_all.hpp>

#include "sensorring/math/Math.hpp"

using eduart::math::eulerDegreesFromRotationMatrix;
using eduart::math::eulerRadiansFromRotationMatrix;
using eduart::math::Matrix3;
using eduart::math::rotMatrixFromEulerDegrees;
using eduart::math::rotMatrixFromEulerRadians;
using eduart::math::Vector3;

TEST_CASE("Euler/rotation matrix round-trip in degrees", "[Math]") {
  Vector3 euler_deg{};
  euler_deg.x() = 30.0;
  euler_deg.y() = -45.0;
  euler_deg.z() = 90.0;

  const Matrix3 R          = rotMatrixFromEulerDegrees(euler_deg);
  const Vector3 euler_back = eulerDegreesFromRotationMatrix(R);

  REQUIRE(euler_back.x() == Catch::Approx(euler_deg.x()).margin(1e-6));
  REQUIRE(euler_back.y() == Catch::Approx(euler_deg.y()).margin(1e-6));
  REQUIRE(euler_back.z() == Catch::Approx(euler_deg.z()).margin(1e-6));
}

TEST_CASE("Euler/rotation matrix round-trip in radians", "[Math]") {
  Vector3 euler_rad{};
  euler_rad.x() = 0.5;
  euler_rad.y() = -0.3;
  euler_rad.z() = 1.2;

  const Matrix3 R          = rotMatrixFromEulerRadians(euler_rad);
  const Vector3 euler_back = eulerRadiansFromRotationMatrix(R);

  REQUIRE(euler_back.x() == Catch::Approx(euler_rad.x()).margin(1e-6));
  REQUIRE(euler_back.y() == Catch::Approx(euler_rad.y()).margin(1e-6));
  REQUIRE(euler_back.z() == Catch::Approx(euler_rad.z()).margin(1e-6));
}

TEST_CASE("Known rotation matrices in degrees", "[Math]") {
  Vector3 euler_deg{};

  SECTION("90 degree rotation about X") {
    euler_deg.x() = 90.0;
    euler_deg.y() = 0.0;
    euler_deg.z() = 0.0;

    const Matrix3 R          = rotMatrixFromEulerDegrees(euler_deg);
    const Vector3 euler_back = eulerDegreesFromRotationMatrix(R);

    REQUIRE(euler_back.x() == Catch::Approx(90.0).margin(1e-6));
    REQUIRE(euler_back.y() == Catch::Approx(0.0).margin(1e-6));
    REQUIRE(euler_back.z() == Catch::Approx(0.0).margin(1e-6));
  }

  SECTION("90 degree rotation about Y") {
    euler_deg.x() = 0.0;
    euler_deg.y() = 90.0;
    euler_deg.z() = 0.0;

    const Matrix3 R          = rotMatrixFromEulerDegrees(euler_deg);
    const Vector3 euler_back = eulerDegreesFromRotationMatrix(R);

    REQUIRE(euler_back.x() == Catch::Approx(0.0).margin(1e-6));
    REQUIRE(euler_back.y() == Catch::Approx(90.0).margin(1e-6));
    REQUIRE(euler_back.z() == Catch::Approx(0.0).margin(1e-6));
  }

  SECTION("90 degree rotation about Z") {
    euler_deg.x() = 0.0;
    euler_deg.y() = 0.0;
    euler_deg.z() = 90.0;

    const Matrix3 R          = rotMatrixFromEulerDegrees(euler_deg);
    const Vector3 euler_back = eulerDegreesFromRotationMatrix(R);

    REQUIRE(euler_back.x() == Catch::Approx(0.0).margin(1e-6));
    REQUIRE(euler_back.y() == Catch::Approx(0.0).margin(1e-6));
    REQUIRE(euler_back.z() == Catch::Approx(90.0).margin(1e-6));
  }
}
