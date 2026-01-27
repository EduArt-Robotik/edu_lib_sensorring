#include <catch2/catch_all.hpp>

#include "sensorring/math/Vector3.hpp"

using eduart::math::Vector3;

TEST_CASE("Vector3 default construction and accessors", "[Vector3]") {
  Vector3 v{};

  v.x() = 1.0;
  v.y() = -2.0;
  v.z() = 3.5;

  REQUIRE(v.x() == Catch::Approx(1.0));
  REQUIRE(v.y() == Catch::Approx(-2.0));
  REQUIRE(v.z() == Catch::Approx(3.5));

  REQUIRE(v[0] == Catch::Approx(1.0));
  REQUIRE(v[1] == Catch::Approx(-2.0));
  REQUIRE(v[2] == Catch::Approx(3.5));
}

TEST_CASE("Vector3 addition and subtraction", "[Vector3]") {
  Vector3 a{};
  a.x() = 1.0;
  a.y() = 2.0;
  a.z() = 3.0;

  Vector3 b{};
  b.x() = -4.0;
  b.y() = 5.0;
  b.z() = -6.0;

  SECTION("operator+") {
    const auto c = a + b;
    REQUIRE(c.x() == Catch::Approx(-3.0));
    REQUIRE(c.y() == Catch::Approx(7.0));
    REQUIRE(c.z() == Catch::Approx(-3.0));
  }

  SECTION("operator-") {
    const auto c = a - b;
    REQUIRE(c.x() == Catch::Approx(5.0));
    REQUIRE(c.y() == Catch::Approx(-3.0));
    REQUIRE(c.z() == Catch::Approx(9.0));
  }

  SECTION("operator+=") {
    a += b;
    REQUIRE(a.x() == Catch::Approx(-3.0));
    REQUIRE(a.y() == Catch::Approx(7.0));
    REQUIRE(a.z() == Catch::Approx(-3.0));
  }

  SECTION("operator-=") {
    a -= b;
    REQUIRE(a.x() == Catch::Approx(5.0));
    REQUIRE(a.y() == Catch::Approx(-3.0));
    REQUIRE(a.z() == Catch::Approx(9.0));
  }
}

TEST_CASE("Vector3 scalar multiplication and division", "[Vector3]") {
  Vector3 v{};
  v.x() = 1.5;
  v.y() = -2.0;
  v.z() = 4.0;

  SECTION("operator* and operator*=") {
    const auto w = v * 2.0;
    REQUIRE(w.x() == Catch::Approx(3.0));
    REQUIRE(w.y() == Catch::Approx(-4.0));
    REQUIRE(w.z() == Catch::Approx(8.0));

    v *= -1.0;
    REQUIRE(v.x() == Catch::Approx(-1.5));
    REQUIRE(v.y() == Catch::Approx(2.0));
    REQUIRE(v.z() == Catch::Approx(-4.0));
  }

  SECTION("operator/ and operator/=") {
    const auto w = v / 2.0;
    REQUIRE(w.x() == Catch::Approx(0.75));
    REQUIRE(w.y() == Catch::Approx(-1.0));
    REQUIRE(w.z() == Catch::Approx(2.0));

    v /= 4.0;
    REQUIRE(v.x() == Catch::Approx(0.375));
    REQUIRE(v.y() == Catch::Approx(-0.5));
    REQUIRE(v.z() == Catch::Approx(1.0));
  }
}

TEST_CASE("Vector3 magnitude", "[Vector3]") {
  SECTION("zero vector") {
    Vector3 v{};
    v.x() = 0.0;
    v.y() = 0.0;
    v.z() = 0.0;
    REQUIRE(v.abs() == Catch::Approx(0.0));
  }

  SECTION("positive components") {
    Vector3 v{};
    v.x() = 3.0;
    v.y() = 4.0;
    v.z() = 12.0;
    REQUIRE(v.abs() == Catch::Approx(13.0));
  }

  SECTION("negative components") {
    Vector3 v{};
    v.x() = -3.0;
    v.y() = -4.0;
    v.z() = -12.0;
    REQUIRE(v.abs() == Catch::Approx(13.0));
  }
}
