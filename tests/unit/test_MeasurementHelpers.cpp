#include <catch2/catch_all.hpp>

#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"

using namespace eduart::sensorring::measurement;

// ---------------------------------------------------------------------------
// TemperatureImage::toGrayscale
// ---------------------------------------------------------------------------

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
  img.data[0] = 10.0; // below min
  img.data[1] = 50.0; // above max

  auto gray = img.toGrayscale(20.0, 40.0);

  REQUIRE(gray.data[0] == 0);
  REQUIRE(gray.data[1] == 255);
}

TEST_CASE("TemperatureImage::toGrayscale with zero range returns empty image", "[TemperatureImage]") {
  TemperatureImage img;
  img.data[0] = 25.0;
  img.data[1] = 25.0;

  auto gray = img.toGrayscale(25.0, 25.0);

  // delta==0: all pixels should be 0 (default)
  REQUIRE(gray.data[0] == 0);
  REQUIRE(gray.data[1] == 0);
}

TEST_CASE("TemperatureImage::toGrayscale auto range", "[TemperatureImage]") {
  TemperatureImage img;
  // Set all to 0 first (default), then specific values
  img.data.fill(25.0);
  img.data[0] = 20.0; // min
  img.data[1] = 40.0; // max

  auto gray = img.toGrayscale();

  REQUIRE(gray.data[0] == 0);   // min → 0
  REQUIRE(gray.data[1] == 255); // max → 255
  // 25.0 is (25-20)/(40-20)*255 = 63.75 → 64
  REQUIRE(gray.data[2] == 64);
}

// ---------------------------------------------------------------------------
// TemperatureImage::toFalseColor
// ---------------------------------------------------------------------------

TEST_CASE("TemperatureImage::toFalseColor returns RGB data", "[TemperatureImage]") {
  TemperatureImage img;
  img.data.fill(20.0);
  img.data[0] = 20.0;
  img.data[1] = 40.0;

  auto fc = img.toFalseColor(20.0, 40.0);

  // Pixel 0 maps to grayscale 0 → Iron[0]
  // Pixel 1 maps to grayscale 255 → Iron[255]
  // Just verify they are different (iron palette maps cold→dark, hot→bright)
  bool different = (fc.data[0][0] != fc.data[1][0]) ||
                   (fc.data[0][1] != fc.data[1][1]) ||
                   (fc.data[0][2] != fc.data[1][2]);
  REQUIRE(different);
}

TEST_CASE("TemperatureImage::toFalseColor auto range", "[TemperatureImage]") {
  TemperatureImage img;
  img.data.fill(30.0);
  img.data[0] = 20.0;
  img.data[1] = 40.0;

  auto fc = img.toFalseColor();

  // Same logic — just check it runs and produces different values
  bool different = (fc.data[0][0] != fc.data[1][0]) ||
                   (fc.data[0][1] != fc.data[1][1]) ||
                   (fc.data[0][2] != fc.data[1][2]);
  REQUIRE(different);
}

// ---------------------------------------------------------------------------
// PointCloud::combine
// ---------------------------------------------------------------------------

TEST_CASE("PointCloud::combine merges multiple clouds", "[PointCloud]") {
  PointCloud a, b;
  a.data.push_back({ { 1.0, 0.0, 0.0 }, 1.0, 0.01, 0 });
  a.data.push_back({ { 2.0, 0.0, 0.0 }, 2.0, 0.02, 0 });
  b.data.push_back({ { 3.0, 0.0, 0.0 }, 3.0, 0.03, 1 });

  auto combined = PointCloud::combine({ a, b });

  REQUIRE(combined.data.size() == 3);
  REQUIRE(combined.data[0].point.x() == Catch::Approx(1.0));
  REQUIRE(combined.data[1].point.x() == Catch::Approx(2.0));
  REQUIRE(combined.data[2].point.x() == Catch::Approx(3.0));
}

TEST_CASE("PointCloud::combine with empty vector", "[PointCloud]") {
  auto combined = PointCloud::combine({});
  REQUIRE(combined.data.empty());
}

TEST_CASE("PointCloud::combine with one cloud", "[PointCloud]") {
  PointCloud a;
  a.data.push_back({ { 5.0, 6.0, 7.0 }, 5.0, 0.1, 0 });

  auto combined = PointCloud::combine({ a });

  REQUIRE(combined.data.size() == 1);
  REQUIRE(combined.data[0].point.x() == Catch::Approx(5.0));
}

// ---------------------------------------------------------------------------
// DepthMeasurement::combinePointClouds
// ---------------------------------------------------------------------------

TEST_CASE("DepthMeasurement::combinePointClouds uses transformed clouds", "[DepthMeasurement]") {
  DepthMeasurement m0, m1;
  m0.point_cloud.data.push_back({ { 1.0, 0.0, 0.0 }, 1.0, 0.01, 0 });
  m0.transformed_point_cloud.data.push_back({ { 10.0, 0.0, 0.0 }, 1.0, 0.01, 0 });
  m1.point_cloud.data.push_back({ { 2.0, 0.0, 0.0 }, 2.0, 0.02, 1 });
  m1.transformed_point_cloud.data.push_back({ { 20.0, 0.0, 0.0 }, 2.0, 0.02, 1 });

  auto combined = DepthMeasurement::combinePointClouds({ m0, m1 });

  // Should use transformed_point_cloud, not point_cloud
  REQUIRE(combined.data.size() == 2);
  REQUIRE(combined.data[0].point.x() == Catch::Approx(10.0));
  REQUIRE(combined.data[1].point.x() == Catch::Approx(20.0));
}

TEST_CASE("DepthMeasurement::combinePointClouds with empty vector", "[DepthMeasurement]") {
  auto combined = DepthMeasurement::combinePointClouds({});
  REQUIRE(combined.data.empty());
}
