#include <catch2/catch_all.hpp>

#include "sensorring/measurement/ThermalMeasurement.hpp"

using namespace eduart::sensorring::measurement;

TEST_CASE("TemperatureImage::toGrayscale with explicit range", "[TemperatureImage]") {
  TemperatureImage img;
  img.data[0] = 20.0;
  img.data[1] = 30.0;
  img.data[2] = 40.0;

  auto gray = img.toGrayscale(20.0, 40.0);

  REQUIRE(gray.data[0] == 0);
  REQUIRE(gray.data[1] == 128);
  REQUIRE(gray.data[2] == 255);
}

TEST_CASE("TemperatureImage::toGrayscale clamps out-of-range values", "[TemperatureImage]") {
  TemperatureImage img;
  img.data[0] = 10.0;
  img.data[1] = 50.0;

  auto gray = img.toGrayscale(20.0, 40.0);

  REQUIRE(gray.data[0] == 0);
  REQUIRE(gray.data[1] == 255);
}

TEST_CASE("TemperatureImage::toGrayscale with zero range returns empty image", "[TemperatureImage]") {
  TemperatureImage img;
  img.data[0] = 25.0;
  img.data[1] = 25.0;

  auto gray = img.toGrayscale(25.0, 25.0);

  REQUIRE(gray.data[0] == 0);
  REQUIRE(gray.data[1] == 0);
}

TEST_CASE("TemperatureImage::toGrayscale auto range", "[TemperatureImage]") {
  TemperatureImage img;
  img.data.fill(25.0);
  img.data[0] = 20.0;
  img.data[1] = 40.0;

  auto gray = img.toGrayscale();

  REQUIRE(gray.data[0] == 0);
  REQUIRE(gray.data[1] == 255);
  REQUIRE(gray.data[2] == 64);
}

TEST_CASE("TemperatureImage::toFalseColor returns RGB data", "[TemperatureImage]") {
  TemperatureImage img;
  img.data.fill(20.0);
  img.data[0] = 20.0;
  img.data[1] = 40.0;

  auto fc = img.toFalseColor(20.0, 40.0);

  bool different = (fc.data[0][0] != fc.data[1][0]) || (fc.data[0][1] != fc.data[1][1]) || (fc.data[0][2] != fc.data[1][2]);
  REQUIRE(different);
}

TEST_CASE("TemperatureImage::toFalseColor auto range", "[TemperatureImage]") {
  TemperatureImage img;
  img.data.fill(30.0);
  img.data[0] = 20.0;
  img.data[1] = 40.0;

  auto fc = img.toFalseColor();

  bool different = (fc.data[0][0] != fc.data[1][0]) || (fc.data[0][1] != fc.data[1][1]) || (fc.data[0][2] != fc.data[1][2]);
  REQUIRE(different);
}
