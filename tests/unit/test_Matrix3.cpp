#include <catch2/catch_all.hpp>

#include "sensorring/math/Matrix3.hpp"

using eduart::sensorring::math::Matrix3;
using eduart::sensorring::math::Vector3;

namespace {

Matrix3 makeIdentity() {
  Matrix3 m{};
  m[0].x() = 1.0;
  m[0].y() = 0.0;
  m[0].z() = 0.0;
  m[1].x() = 0.0;
  m[1].y() = 1.0;
  m[1].z() = 0.0;
  m[2].x() = 0.0;
  m[2].y() = 0.0;
  m[2].z() = 1.0;
  return m;
}

} // namespace

TEST_CASE("Matrix3 construction and row access", "[Matrix3]") {
  Matrix3 m{};
  m[0].x() = 1.0;
  m[0].y() = 2.0;
  m[0].z() = 3.0;
  m[1].x() = 4.0;
  m[1].y() = 5.0;
  m[1].z() = 6.0;
  m[2].x() = 7.0;
  m[2].y() = 8.0;
  m[2].z() = 9.0;

  REQUIRE(m[0].x() == Catch::Approx(1.0));
  REQUIRE(m[0].y() == Catch::Approx(2.0));
  REQUIRE(m[0].z() == Catch::Approx(3.0));
  REQUIRE(m[1].x() == Catch::Approx(4.0));
  REQUIRE(m[1].y() == Catch::Approx(5.0));
  REQUIRE(m[1].z() == Catch::Approx(6.0));
  REQUIRE(m[2].x() == Catch::Approx(7.0));
  REQUIRE(m[2].y() == Catch::Approx(8.0));
  REQUIRE(m[2].z() == Catch::Approx(9.0));
}

TEST_CASE("Matrix3 identity behavior", "[Matrix3]") {
  const auto I = makeIdentity();

  Matrix3 m{};
  m[0].x() = 1.0;
  m[0].y() = 2.0;
  m[0].z() = 3.0;
  m[1].x() = 4.0;
  m[1].y() = 5.0;
  m[1].z() = 6.0;
  m[2].x() = 7.0;
  m[2].y() = 8.0;
  m[2].z() = 9.0;

  SECTION("I * m == m") {
    const auto result = I * m;
    for (std::size_t r = 0; r < 3; ++r) {
      for (std::size_t c = 0; c < 3; ++c) {
        REQUIRE(result[r][c] == Catch::Approx(m[r][c]));
      }
    }
  }

  SECTION("m * I == m") {
    const auto result = m * I;
    for (std::size_t r = 0; r < 3; ++r) {
      for (std::size_t c = 0; c < 3; ++c) {
        REQUIRE(result[r][c] == Catch::Approx(m[r][c]));
      }
    }
  }
}

TEST_CASE("Matrix3 * Vector3 multiplication", "[Matrix3]") {
  Matrix3 m{};
  m[0].x() = 1.0;
  m[0].y() = 2.0;
  m[0].z() = 3.0;
  m[1].x() = 0.0;
  m[1].y() = -1.0;
  m[1].z() = 4.0;
  m[2].x() = 2.0;
  m[2].y() = 0.5;
  m[2].z() = -2.0;

  Vector3 v{};
  v.x() = 1.0;
  v.y() = -2.0;
  v.z() = 0.5;

  const auto result = m * v;

  REQUIRE(result.x() == Catch::Approx(1.0 * 1.0 + 2.0 * -2.0 + 3.0 * 0.5));
  REQUIRE(result.y() == Catch::Approx(0.0 * 1.0 + -1.0 * -2.0 + 4.0 * 0.5));
  REQUIRE(result.z() == Catch::Approx(2.0 * 1.0 + 0.5 * -2.0 + -2.0 * 0.5));
}

TEST_CASE("Matrix3 scalar operations", "[Matrix3]") {
  SECTION("operator* and operator*=") {
    Matrix3 m{};
    m[0].x() = 1.0;
    m[0].y() = 2.0;
    m[0].z() = 3.0;
    m[1].x() = 4.0;
    m[1].y() = 5.0;
    m[1].z() = 6.0;
    m[2].x() = 7.0;
    m[2].y() = 8.0;
    m[2].z() = 9.0;

    const auto scaled = m * 2.0;
    // Verify scaled values against original m
    REQUIRE(scaled[0].x() == Catch::Approx(2.0));
    REQUIRE(scaled[0].y() == Catch::Approx(4.0));
    REQUIRE(scaled[0].z() == Catch::Approx(6.0));
    REQUIRE(scaled[1].x() == Catch::Approx(8.0));
    REQUIRE(scaled[1].y() == Catch::Approx(10.0));
    REQUIRE(scaled[1].z() == Catch::Approx(12.0));
    REQUIRE(scaled[2].x() == Catch::Approx(14.0));
    REQUIRE(scaled[2].y() == Catch::Approx(16.0));
    REQUIRE(scaled[2].z() == Catch::Approx(18.0));

    m *= 0.5;
    REQUIRE(m[0].x() == Catch::Approx(0.5));
    REQUIRE(m[0].y() == Catch::Approx(1.0));
    REQUIRE(m[0].z() == Catch::Approx(1.5));
    REQUIRE(m[1].x() == Catch::Approx(2.0));
    REQUIRE(m[1].y() == Catch::Approx(2.5));
    REQUIRE(m[1].z() == Catch::Approx(3.0));
    REQUIRE(m[2].x() == Catch::Approx(3.5));
    REQUIRE(m[2].y() == Catch::Approx(4.0));
    REQUIRE(m[2].z() == Catch::Approx(4.5));
  }

  SECTION("operator/ and operator/=") {
    Matrix3 m{};
    m[0].x() = 1.0;
    m[0].y() = 2.0;
    m[0].z() = 3.0;
    m[1].x() = 4.0;
    m[1].y() = 5.0;
    m[1].z() = 6.0;
    m[2].x() = 7.0;
    m[2].y() = 8.0;
    m[2].z() = 9.0;

    const auto scaled = m / 2.0;
    // Verify scaled values against original m
    REQUIRE(scaled[0].x() == Catch::Approx(0.5));
    REQUIRE(scaled[0].y() == Catch::Approx(1.0));
    REQUIRE(scaled[0].z() == Catch::Approx(1.5));
    REQUIRE(scaled[1].x() == Catch::Approx(2.0));
    REQUIRE(scaled[1].y() == Catch::Approx(2.5));
    REQUIRE(scaled[1].z() == Catch::Approx(3.0));
    REQUIRE(scaled[2].x() == Catch::Approx(3.5));
    REQUIRE(scaled[2].y() == Catch::Approx(4.0));
    REQUIRE(scaled[2].z() == Catch::Approx(4.5));

    m /= 2.0;
    REQUIRE(m[0].x() == Catch::Approx(0.5));
    REQUIRE(m[0].y() == Catch::Approx(1.0));
    REQUIRE(m[0].z() == Catch::Approx(1.5));
    REQUIRE(m[1].x() == Catch::Approx(2.0));
    REQUIRE(m[1].y() == Catch::Approx(2.5));
    REQUIRE(m[1].z() == Catch::Approx(3.0));
    REQUIRE(m[2].x() == Catch::Approx(3.5));
    REQUIRE(m[2].y() == Catch::Approx(4.0));
    REQUIRE(m[2].z() == Catch::Approx(4.5));
  }
}
