#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "interface/ComInterface.hpp"
#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/measurement/PointCloud.hpp"

using eduart::sensorring::device::DepthSensor;
using eduart::sensorring::device::DepthSensorConfig;
using eduart::sensorring::device::DepthSensorParams;
using eduart::sensorring::measurement::PointCloud;

static constexpr double fov_x              = 45.0;
static constexpr double fov_y              = 45.0;
static constexpr unsigned int resolution_x = 8;
static constexpr unsigned int resolution_y = 8;
static constexpr double expected_lut_x_8[] = { -0.3624, -0.2589, -0.1553, -0.0518, 0.0518, 0.1553, 0.2589, 0.3624 };
static constexpr double expected_lut_y_8[] = { -0.3624, -0.2589, -0.1553, -0.0518, 0.0518, 0.1553, 0.2589, 0.3624 };

namespace {

class NullComInterface : public eduart::sensorring::com::ComInterface {
public:
  NullComInterface()
      : ComInterface(eduart::sensorring::com::ComInterfaceID{}) {}
  bool send(eduart::sensorring::com::ComEndpoint, std::uint8_t, const std::vector<std::uint8_t>&) override { return true; }
  bool openInterface() override { return true; }
  bool closeInterface() override { return true; }
  bool repairInterface() override { return true; }

protected:
  bool listener() override { return true; }
};

NullComInterface& getNullInterface() {
  static NullComInterface instance;
  return instance;
}

} // namespace

class MockDepthSensor : public DepthSensor {
public:
  MockDepthSensor(DepthSensorConfig config = { 45.0, 45.0, 8, 8 })
      : DepthSensor(eduart::sensorring::device::DeviceID({ eduart::sensorring::device::DeviceType::VL53L8CX, 0 }), &getNullInterface(), eduart::sensorring::com::ComEndpoint{}, true, config) {}

  const DepthSensorParams& getParams() const override { return _params; }

  void publishMeasurement() override {}

  void comCallback(const eduart::sensorring::com::ComEndpoint, std::uint8_t, const std::vector<std::uint8_t>&) override {}

  const std::vector<double>& lutX() const { return _lut_x; }
  const std::vector<double>& lutY() const { return _lut_y; }
  const std::vector<double>& lutZ() const { return _lut_z; }

  using DepthSensor::processRawMeasurement;

  void toPointCloud(const std::vector<double>& lut_x, const std::vector<double>& lut_y, PointCloud& pcl) {
    _lut_x        = lut_x;
    _lut_y        = lut_y;
    _config.res_x = static_cast<unsigned int>(lut_x.size());
    _config.res_y = static_cast<unsigned int>(lut_y.size());
    processRawMeasurement(pcl);
  }

private:
  DepthSensorParams _params;
};

TEST_CASE("DepthSensor point cloud operations", "[DepthSensor]") {

  MockDepthSensor m({ fov_x, fov_y, resolution_x, resolution_y });

  SECTION("DepthSensor lookup table creation") {
    REQUIRE(m.lutX().size() == resolution_x);
    REQUIRE(m.lutY().size() == resolution_y);

    for (unsigned int i = 0; i < resolution_x; ++i) {
      CHECK(m.lutX()[i] == Catch::Approx(expected_lut_x_8[i]).margin(1e-4)); // Reference values have 4 decimal places -> margin of 1e-4
    }

    for (unsigned int j = 0; j < resolution_y; ++j) {
      CHECK(m.lutY()[j] == Catch::Approx(expected_lut_y_8[j]).margin(1e-4)); // Reference values have 4 decimal places -> margin of 1e-4
    }
  }

  SECTION("DepthSensor raw measurement to point cloud transformation") {
    std::vector<double> lut_x{ -1.0, 1.0 };
    std::vector<double> lut_y{ -2.0, 2.0 };

    PointCloud pcl;
    pcl.data.resize(4); // Has to match LUT size otherwise processRawMeasurement throws
    pcl.data[0].raw_distance = 1.0;
    pcl.data[1].raw_distance = 2.0;
    pcl.data[2].raw_distance = -1.0;
    pcl.data[3].raw_distance = 3.0;
    REQUIRE_NOTHROW(m.toPointCloud(lut_x, lut_y, pcl));

    REQUIRE(pcl.data.size() == 4u);

    REQUIRE(pcl.data[0].point.x() == Catch::Approx(-1.0));
    REQUIRE(pcl.data[0].point.y() == Catch::Approx(-2.0));
    REQUIRE(pcl.data[0].point.z() == Catch::Approx(1.0));
    REQUIRE(pcl.data[0].raw_distance == Catch::Approx(1.0));
    REQUIRE(pcl.data[0].sigma == Catch::Approx(0.0));

    REQUIRE(pcl.data[1].point.x() == Catch::Approx(2.0));
    REQUIRE(pcl.data[1].point.y() == Catch::Approx(-4.0));
    REQUIRE(pcl.data[1].point.z() == Catch::Approx(2.0));
    REQUIRE(pcl.data[1].raw_distance == Catch::Approx(2.0));
    REQUIRE(pcl.data[1].sigma == Catch::Approx(0.0));

    REQUIRE(std::isnan(pcl.data[2].point.x()));
    REQUIRE(std::isnan(pcl.data[2].point.y()));
    REQUIRE(std::isnan(pcl.data[2].point.z()));
    REQUIRE(std::isnan(pcl.data[2].raw_distance));
    REQUIRE(std::isnan(pcl.data[2].sigma));

    REQUIRE(pcl.data[3].point.x() == Catch::Approx(3.0));
    REQUIRE(pcl.data[3].point.y() == Catch::Approx(6.0));
    REQUIRE(pcl.data[3].point.z() == Catch::Approx(3.0));
    REQUIRE(pcl.data[3].raw_distance == Catch::Approx(3.0));
    REQUIRE(pcl.data[3].sigma == Catch::Approx(0.0));

    pcl.data.resize(64); // Has to match LUT size otherwise processRawMeasurement throws
    REQUIRE_THROWS_AS(m.toPointCloud(lut_x, lut_y, pcl), std::runtime_error);
  }
}

TEST_CASE("DepthSensor Config invert_x_lut", "[DepthSensor]") {

  static constexpr double fov_x              = 45.0;
  static constexpr double fov_y              = 45.0;
  static constexpr unsigned int resolution_x = 8;
  static constexpr unsigned int resolution_y = 8;

  DepthSensorConfig config;
  config.fov_x_deg    = fov_x;
  config.fov_y_deg    = fov_y;
  config.res_x        = resolution_x;
  config.res_y        = resolution_y;
  config.invert_x_lut = true;
  MockDepthSensor m(config);

  SECTION("Inverted X LUT values are negated compared to default") {
    REQUIRE(m.lutX().size() == resolution_x);
    for (unsigned int i = 0; i < resolution_x; ++i) {
      CHECK(m.lutX()[i] == Catch::Approx(-expected_lut_x_8[i]).margin(1e-4));
    }
    // Y LUT should remain unchanged
    for (unsigned int j = 0; j < resolution_y; ++j) {
      CHECK(m.lutY()[j] == Catch::Approx(expected_lut_y_8[j]).margin(1e-4));
    }
  }

  SECTION("Inverted X LUT affects point cloud X coordinates") {
    DepthSensorConfig cfg;
    cfg.invert_x_lut = false;
    cfg.fov_x_deg    = fov_x;
    cfg.fov_y_deg    = fov_y;
    cfg.res_x        = resolution_x;
    cfg.res_y        = resolution_y;
    MockDepthSensor default_sensor(cfg);

    // The inverted sensor's X LUT values should be negated
    for (unsigned int i = 0; i < resolution_x; ++i) {
      CHECK(m.lutX()[i] == Catch::Approx(-default_sensor.lutX()[i]).margin(1e-10));
    }
  }
}

TEST_CASE("DepthSensor Config invert_y_lut", "[DepthSensor]") {

  static constexpr double fov_x              = 45.0;
  static constexpr double fov_y              = 45.0;
  static constexpr unsigned int resolution_x = 8;
  static constexpr unsigned int resolution_y = 8;

  DepthSensorConfig config;
  config.invert_y_lut = true;
  config.fov_x_deg    = fov_x;
  config.fov_y_deg    = fov_y;
  config.res_x        = resolution_x;
  config.res_y        = resolution_y;
  MockDepthSensor m(config);

  SECTION("Inverted Y LUT values are negated compared to default") {
    REQUIRE(m.lutY().size() == resolution_y);
    for (unsigned int j = 0; j < resolution_y; ++j) {
      CHECK(m.lutY()[j] == Catch::Approx(-expected_lut_y_8[j]).margin(1e-4));
    }
    // X LUT should remain unchanged
    for (unsigned int i = 0; i < resolution_x; ++i) {
      CHECK(m.lutX()[i] == Catch::Approx(expected_lut_x_8[i]).margin(1e-4));
    }
  }

  SECTION("Inverted Y LUT affects point cloud Y coordinates") {
    DepthSensorConfig cfg;
    cfg.invert_y_lut = false;
    cfg.fov_x_deg    = fov_x;
    cfg.fov_y_deg    = fov_y;
    cfg.res_x        = resolution_x;
    cfg.res_y        = resolution_y;
    MockDepthSensor default_sensor(cfg);

    // The inverted sensor's Y LUT values should be negated
    for (unsigned int j = 0; j < resolution_y; ++j) {
      CHECK(m.lutY()[j] == Catch::Approx(-default_sensor.lutY()[j]).margin(1e-10));
    }
  }
}

TEST_CASE("DepthSensor Config reports_perpendicular_distance", "[DepthSensor]") {

  static constexpr double fov_x              = 45.0;
  static constexpr double fov_y              = 45.0;
  static constexpr unsigned int resolution_x = 2;
  static constexpr unsigned int resolution_y = 2;

  SECTION("Default config (perpendicular distance) does not create Z LUT") {
    DepthSensorConfig config;
    config.reports_perpendicular_distance = true;
    config.fov_x_deg                      = fov_x;
    config.fov_y_deg                      = fov_y;
    config.res_x                          = resolution_x;
    config.res_y                          = resolution_y;
    MockDepthSensor m(config);
    REQUIRE(m.lutZ().empty());
  }

  SECTION("Non-perpendicular distance creates Z LUT with correction factors") {
    DepthSensorConfig config;
    config.reports_perpendicular_distance = false;
    config.fov_x_deg                      = fov_x;
    config.fov_y_deg                      = fov_y;
    config.res_x                          = resolution_x;
    config.res_y                          = resolution_y;
    MockDepthSensor m(config);

    REQUIRE(m.lutZ().size() == resolution_x * resolution_y);

    // Each Z LUT entry should be 1/sqrt(1 + lut_x[i]^2 + lut_y[j]^2)
    for (unsigned int j = 0; j < resolution_y; ++j) {
      for (unsigned int i = 0; i < resolution_x; ++i) {
        double expected = 1.0 / std::sqrt(1.0 + m.lutX()[i] * m.lutX()[i] + m.lutY()[j] * m.lutY()[j]);
        CHECK(m.lutZ()[j * resolution_x + i] == Catch::Approx(expected).margin(1e-10));
      }
    }

    // All correction factors should be in (0, 1] range
    for (const auto& z : m.lutZ()) {
      CHECK(z > 0.0);
      CHECK(z <= 1.0);
    }
  }

  SECTION("Non-perpendicular distance corrects point cloud Z values") {
    DepthSensorConfig config;
    config.reports_perpendicular_distance = false;
    config.fov_x_deg                      = fov_x;
    config.fov_y_deg                      = fov_y;
    config.res_x                          = resolution_x;
    config.res_y                          = resolution_y;
    MockDepthSensor m(config);

    PointCloud pcl;
    pcl.data.resize(resolution_x * resolution_y);
    double raw_dist = 5.0;
    for (auto& point : pcl.data) {
      point.raw_distance = raw_dist;
    }

    m.processRawMeasurement(pcl);

    // With correction, distance = raw_distance * lut_z[i], which is less than raw_distance
    // z coordinate equals the corrected distance, x = distance * lut_x, y = distance * lut_y
    for (unsigned int j = 0; j < resolution_y; ++j) {
      for (unsigned int i = 0; i < resolution_x; ++i) {
        unsigned int idx          = j * resolution_x + i;
        double corrected_distance = raw_dist * m.lutZ()[idx];
        CHECK(pcl.data[idx].point.z() == Catch::Approx(corrected_distance).margin(1e-10));
        CHECK(pcl.data[idx].point.x() == Catch::Approx(corrected_distance * m.lutX()[i]).margin(1e-10));
        CHECK(pcl.data[idx].point.y() == Catch::Approx(corrected_distance * m.lutY()[j]).margin(1e-10));
        // Raw distance should remain unchanged
        CHECK(pcl.data[idx].raw_distance == Catch::Approx(raw_dist));
      }
    }
  }

  SECTION("Perpendicular distance does not correct point cloud Z values") {
    DepthSensorConfig config;
    config.reports_perpendicular_distance = true;
    config.fov_x_deg                      = fov_x;
    config.fov_y_deg                      = fov_y;
    config.res_x                          = resolution_x;
    config.res_y                          = resolution_y;
    MockDepthSensor m(config);

    PointCloud pcl;
    pcl.data.resize(resolution_x * resolution_y);
    double raw_dist = 5.0;
    for (auto& point : pcl.data) {
      point.raw_distance = raw_dist;
    }

    m.processRawMeasurement(pcl);

    // Without correction, z equals raw_distance directly
    for (unsigned int j = 0; j < resolution_y; ++j) {
      for (unsigned int i = 0; i < resolution_x; ++i) {
        unsigned int idx = j * resolution_x + i;
        CHECK(pcl.data[idx].point.z() == Catch::Approx(raw_dist).margin(1e-10));
        CHECK(pcl.data[idx].point.x() == Catch::Approx(raw_dist * m.lutX()[i]).margin(1e-10));
        CHECK(pcl.data[idx].point.y() == Catch::Approx(raw_dist * m.lutY()[j]).margin(1e-10));
      }
    }
  }
}