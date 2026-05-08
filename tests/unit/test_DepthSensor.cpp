#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/measurement/PointCloud.hpp"

using eduart::sensorring::device::DepthSensor;
using eduart::sensorring::measurement::PointCloud;

static constexpr double expected_lut_x_8[] = { 0.3624, 0.2589, 0.1553, 0.0518, -0.0518, -0.1553, -0.2589, -0.3624 };

static constexpr double expected_lut_y_8[] = { 0.3624, 0.2589, 0.1553, 0.0518, -0.0518, -0.1553, -0.2589, -0.3624 };

class MockDepthSensor : public DepthSensor {
public:
  MockDepthSensor(double fov_x = 45.0, double fov_y = 45.0, unsigned int res_x = 8, unsigned int res_y = 8)
      : DepthSensor(fov_x, fov_y, res_x, res_y) {}

  void publishMeasurement() override {}

  const std::vector<double>& lutX() const { return _lut_x; }
  const std::vector<double>& lutY() const { return _lut_y; }

  void toPointCloud(const std::vector<double>& lut_x, const std::vector<double>& lut_y, PointCloud& pcl) { processRawMeasurement(lut_x, lut_y, pcl); }
};

TEST_CASE("DepthSensor point cloud operations", "[DepthSensor]") {

  static constexpr double fov_x              = 45.0;
  static constexpr double fov_y              = 45.0;
  static constexpr unsigned int resolution_x = 8;
  static constexpr unsigned int resolution_y = 8;
  MockDepthSensor m(fov_x, fov_y, resolution_x, resolution_y);

  SECTION("DepthSensor lookup table creation") {
    REQUIRE(m.lutX().size() == resolution_x);
    REQUIRE(m.lutY().size() == resolution_y);

    for (unsigned int i = 0; i < resolution_x; ++i) {
      CHECK(m.lutX()[i] == Catch::Approx(expected_lut_x_8[i]).margin(1e-6));
    }

    for (unsigned int j = 0; j < resolution_y; ++j) {
      CHECK(m.lutY()[j] == Catch::Approx(expected_lut_y_8[j]).margin(1e-6));
    }
  }

  SECTION("DepthSensor raw measurement to point cloud transformation") {
    std::vector<double> lut_x{ -1.0, 1.0 };
    std::vector<double> lut_y{ -2.0, 2.0 };

    PointCloud pcl;
    pcl.data.resize(4);
    pcl.data[0].raw_distance = 1.0;
    pcl.data[1].raw_distance = 2.0;
    pcl.data[2].raw_distance = -1.0;
    pcl.data[3].raw_distance = 3.0;
    m.toPointCloud(lut_x, lut_y, pcl);

    REQUIRE(pcl.data.size() == 4u);

    REQUIRE(pcl.data[0].point.x() == Catch::Approx(-1.0));
    REQUIRE(pcl.data[0].point.y() == Catch::Approx(-2.0));
    REQUIRE(pcl.data[0].point.z() == Catch::Approx(1.0));
    REQUIRE(pcl.data[0].raw_distance == Catch::Approx(1.0));
    REQUIRE(pcl.data[0].sigma == Catch::Approx(0.0));

    REQUIRE(pcl.data[1].point.x() == Catch::Approx(-2.0));
    REQUIRE(pcl.data[1].point.y() == Catch::Approx(4.0));
    REQUIRE(pcl.data[1].point.z() == Catch::Approx(2.0));
    REQUIRE(pcl.data[1].raw_distance == Catch::Approx(2.0));
    REQUIRE(pcl.data[1].sigma == Catch::Approx(0.0));

    REQUIRE(pcl.data[2].point.x() == Catch::Approx(0.0));
    REQUIRE(pcl.data[2].point.y() == Catch::Approx(0.0));
    REQUIRE(pcl.data[2].point.z() == Catch::Approx(0.0));
    REQUIRE(pcl.data[2].raw_distance == Catch::Approx(-1.0));
    REQUIRE(pcl.data[2].sigma == Catch::Approx(-1.0));

    REQUIRE(pcl.data[3].point.x() == Catch::Approx(3.0));
    REQUIRE(pcl.data[3].point.y() == Catch::Approx(6.0));
    REQUIRE(pcl.data[3].point.z() == Catch::Approx(3.0));
    REQUIRE(pcl.data[3].raw_distance == Catch::Approx(3.0));
    REQUIRE(pcl.data[3].sigma == Catch::Approx(0.0));
  }
}