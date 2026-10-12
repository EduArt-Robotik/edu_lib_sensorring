// Copyright (c) 2026 EduArt Robotik GmbH

#include <array>
#include <catch2/catch_all.hpp>
#include <chrono>
#include <memory>
#include <sensorring_transport/Protocol.hpp>
#include <vector>

#include "device/thermal/htpa32/HTPA32_DeviceImpl.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Device.hpp"

namespace eduart {

namespace sensorring {

namespace device {

class HTPA32_DeviceTestAccess {
public:
  static std::unique_ptr<HTPA32_Device> create(com::ComInterface* interface) { return std::unique_ptr<HTPA32_Device>(new HTPA32_Device(HTPA32_Params{}, interface, 0, HTPA32_Device::InterfaceTag{})); }

  static void setEeprom(HTPA32_Device& sensor, const htpa32::HTPA32_Eeprom& eeprom) { sensor._impl->_eeprom = eeprom; }

  static measurement::TemperatureImage processSensorOrder(HTPA32_Device& sensor, const std::vector<std::uint8_t>& payload) {
    const std::uint16_t vdd  = payload[0] | (payload[1] << 8);
    const std::uint16_t ptat = payload[2] | (payload[3] << 8);
    sensor._impl->_ptat      = ptat;
    const auto result        = sensor._impl->processMeasurement(0, payload.data() + 4, sensor._impl->_eeprom, vdd, ptat, NUMBER_OF_PIXEL);
    REQUIRE(result.second == DeviceState::Ok);
    return result.first.temperatures;
  }

  static void setCalibration(HTPA32_Device& sensor, measurement::TemperatureImage calibration) {
    sensor._impl->_calibration_image   = calibration;
    sensor._impl->_calibration_average = calibration.avg();
    sensor._impl->_got_calibration     = true;
  }

  static const measurement::TemperatureImage& calibration(const HTPA32_Device& sensor) { return sensor._impl->_calibration_image; }

  static bool calibrationActive(const HTPA32_Device& sensor) { return sensor._impl->_calibration_active; }

  static void receive(HTPA32_Device& sensor, const std::vector<std::uint8_t>& payload) { sensor.comCallback({}, transport::protocol::htpa32::MEASUREMENT_TRANSMISSION_RESPONSE, payload); }
};

} // namespace device

} // namespace sensorring

} // namespace eduart

using eduart::sensorring::board::Orientation;
using eduart::sensorring::board::SensorBoardParams;
using eduart::sensorring::device::HTPA32_DeviceTestAccess;
using eduart::sensorring::device::htpa32::HTPA32_Eeprom;
using eduart::sensorring::measurement::TemperatureImage;

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

HTPA32_Eeprom makeEeprom() {
  HTPA32_Eeprom eeprom{};
  eeprom.data.ptat_gradient = 1.0F;
  eeprom.data.ptat_th1      = 2800;
  eeprom.data.ptat_th2      = 3200;
  eeprom.data.vddth1        = 3200;
  eeprom.data.vddth2        = 3400;
  eeprom.data.grad_scale    = 4;

  for (std::size_t i = 0; i < NUMBER_OF_PIXEL; ++i) {
    eeprom.data.th_gradient[i] = static_cast<std::int16_t>(i % 3);
    eeprom.data.th_offset[i]   = static_cast<std::int16_t>(i % 7);
  }
  return eeprom;
}

std::vector<std::uint8_t> makePayload(std::uint16_t shift = 0) {
  std::vector<std::uint8_t> payload(4 + 512 + 2 * NUMBER_OF_PIXEL);
  payload[0] = 3300 & 0xff;
  payload[1] = 3300 >> 8;
  payload[2] = 3000 & 0xff;
  payload[3] = 3000 >> 8;
  for (std::size_t i = 0; i < 256; ++i) {
    const auto offset      = static_cast<std::uint16_t>(i);
    payload[4 + i * 2]     = static_cast<std::uint8_t>(offset);
    payload[4 + i * 2 + 1] = static_cast<std::uint8_t>(offset >> 8);
  }
  for (std::size_t i = 0; i < NUMBER_OF_PIXEL; ++i) {
    const auto pixel             = static_cast<std::uint16_t>(2000 + i * 32 + shift);
    payload[4 + 512 + i * 2]     = static_cast<std::uint8_t>(pixel >> 8);
    payload[4 + 512 + i * 2 + 1] = static_cast<std::uint8_t>(pixel);
  }
  return payload;
}

// Explicit historical row mapping, independent of the implementation's block arithmetic.
constexpr std::array<std::size_t, 32> SENSOR_ROWS = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28 };

TemperatureImage expectedImage(const TemperatureImage& sensor_order, Orientation orientation) {
  TemperatureImage expected;
  for (std::size_t row = 0; row < 32; ++row) {
    for (std::size_t col = 0; col < 32; ++col) {
      const auto image_row          = orientation == Orientation::Left ? 31 - row : row;
      const auto image_col          = orientation == Orientation::Left ? 31 - col : col;
      expected.data[row * 32 + col] = sensor_order.data[SENSOR_ROWS[image_row] * 32 + image_col];
    }
  }
  return expected;
}

void checkImage(const TemperatureImage& actual, const TemperatureImage& expected) {
  for (std::size_t i = 0; i < actual.data.size(); ++i) {
    CAPTURE(i);
    CHECK(actual.data[i] == Catch::Approx(expected.data[i]).margin(1e-9));
  }
}

struct ThermalFixture {
  NullComInterface interface;
  SensorBoardParams board;
  std::unique_ptr<eduart::sensorring::device::HTPA32_Device> sensor = HTPA32_DeviceTestAccess::create(&interface);
  std::vector<std::uint8_t> payload                                 = makePayload();

  ThermalFixture() {
    sensor->setBoardContext(&board);
    HTPA32_DeviceTestAccess::setEeprom(*sensor, makeEeprom());
  }

  void receive(const std::vector<std::uint8_t>& data) {
    sensor->clearDataFlag();
    auto ready = sensor->beginMeasurementWait();
    HTPA32_DeviceTestAccess::receive(*sensor, data);
    REQUIRE(ready.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
    REQUIRE(ready.get());
  }

  TemperatureImage setPixelIndexCalibration() {
    auto calibration = HTPA32_DeviceTestAccess::processSensorOrder(*sensor, payload);
    for (std::size_t i = 0; i < calibration.data.size(); ++i) {
      calibration.data[i] -= static_cast<double>(i);
    }
    const double average = calibration.avg();
    HTPA32_DeviceTestAccess::setCalibration(*sensor, calibration);
    TemperatureImage calibrated;
    for (std::size_t i = 0; i < calibrated.data.size(); ++i) {
      calibrated.data[i] = static_cast<double>(i) + average;
    }
    return calibrated;
  }
};

} // namespace

TEST_CASE("HTPA32 reconstructs all sensor sections after EEPROM compensation", "[HTPA32]") {
  ThermalFixture fixture;
  fixture.board.orientation = GENERATE(Orientation::None, Orientation::Right, Orientation::Left);
  const auto sensor_order   = HTPA32_DeviceTestAccess::processSensorOrder(*fixture.sensor, fixture.payload);

  fixture.receive(fixture.payload);

  checkImage(fixture.sensor->getLatestMeasurement().temperatures, expectedImage(sensor_order, fixture.board.orientation));
}

TEST_CASE("HTPA32 without board context reconstructs sections without mounting rotation", "[HTPA32]") {
  ThermalFixture fixture;
  fixture.sensor->setBoardContext(nullptr);
  const auto sensor_order = HTPA32_DeviceTestAccess::processSensorOrder(*fixture.sensor, fixture.payload);

  fixture.receive(fixture.payload);

  checkImage(fixture.sensor->getLatestMeasurement().temperatures, expectedImage(sensor_order, Orientation::None));
}

TEST_CASE("HTPA32 preserves sensor-order calibration and publishes corrected temperatures once", "[HTPA32]") {
  ThermalFixture fixture;
  fixture.board.orientation     = GENERATE(Orientation::None, Orientation::Right, Orientation::Left);
  const auto calibrated         = fixture.setPixelIndexCalibration();
  const auto expected           = expectedImage(calibrated, fixture.board.orientation);
  const auto calibration_before = HTPA32_DeviceTestAccess::calibration(*fixture.sensor);
  unsigned int publications     = 0;
  auto subscription             = fixture.sensor->subscribe([&](const auto& measurement) {
    ++publications;
    checkImage(measurement.temperatures, expected);
  });

  fixture.receive(fixture.payload);
  checkImage(fixture.sensor->getLatestMeasurement().temperatures, expected);
  fixture.sensor->publishMeasurement();
  fixture.sensor->publishMeasurement();

  CHECK(publications == 2);
  CHECK(HTPA32_DeviceTestAccess::calibration(*fixture.sensor).data == calibration_before.data);
}

TEST_CASE("HTPA32 generic visualizations use the corrected temperature layout", "[HTPA32][TemperatureImage]") {
  ThermalFixture fixture;
  fixture.board.orientation = GENERATE(Orientation::None, Orientation::Right, Orientation::Left);
  const auto expected       = expectedImage(fixture.setPixelIndexCalibration(), fixture.board.orientation);
  fixture.receive(fixture.payload);
  const auto& actual = fixture.sensor->getLatestMeasurement().temperatures;

  CHECK(actual.toGrayscale().data == expected.toGrayscale().data);
  CHECK(actual.toFalseColor().data == expected.toFalseColor().data);
  CHECK(actual.toGrayscale(20.0, 1000.0).data == expected.toGrayscale(20.0, 1000.0).data);
  CHECK(actual.toFalseColor(20.0, 1000.0).data == expected.toFalseColor(20.0, 1000.0).data);
}

TEST_CASE("HTPA32 uses current board orientation for each frame without changing calibration", "[HTPA32]") {
  ThermalFixture fixture;
  const auto calibrated         = fixture.setPixelIndexCalibration();
  const auto calibration_before = HTPA32_DeviceTestAccess::calibration(*fixture.sensor);

  for (const auto orientation : { Orientation::Right, Orientation::Left, Orientation::None, Orientation::Left }) {
    fixture.board.orientation = orientation;
    fixture.receive(fixture.payload);
    checkImage(fixture.sensor->getLatestMeasurement().temperatures, expectedImage(calibrated, orientation));
    CHECK(HTPA32_DeviceTestAccess::calibration(*fixture.sensor).data == calibration_before.data);
  }
}

TEST_CASE("HTPA32 collects new calibration in sensor order before correcting each image", "[HTPA32]") {
  ThermalFixture fixture;
  fixture.board.orientation = GENERATE(Orientation::None, Orientation::Right, Orientation::Left);
  const auto first          = HTPA32_DeviceTestAccess::processSensorOrder(*fixture.sensor, fixture.payload);
  const auto second_payload = makePayload(512);
  const auto second         = HTPA32_DeviceTestAccess::processSensorOrder(*fixture.sensor, second_payload);

  for (unsigned int i = 0; i < 6; ++i) {
    fixture.receive(fixture.payload);
  }
  REQUIRE(fixture.sensor->startCalibration(2));
  fixture.receive(fixture.payload);
  REQUIRE(HTPA32_DeviceTestAccess::calibrationActive(*fixture.sensor));
  checkImage(HTPA32_DeviceTestAccess::calibration(*fixture.sensor), first);
  checkImage(fixture.sensor->getLatestMeasurement().temperatures, expectedImage(first, fixture.board.orientation));

  fixture.receive(second_payload);
  REQUIRE_FALSE(HTPA32_DeviceTestAccess::calibrationActive(*fixture.sensor));
  TemperatureImage calibration = first;
  calibration += second;
  calibration /= 2.0;
  checkImage(HTPA32_DeviceTestAccess::calibration(*fixture.sensor), calibration);

  TemperatureImage corrected = second;
  corrected -= calibration;
  corrected += calibration.avg();
  checkImage(fixture.sensor->getLatestMeasurement().temperatures, expectedImage(corrected, fixture.board.orientation));

  fixture.board.orientation = Orientation::Left;
  fixture.receive(second_payload);
  checkImage(fixture.sensor->getLatestMeasurement().temperatures, expectedImage(corrected, Orientation::Left));
  checkImage(HTPA32_DeviceTestAccess::calibration(*fixture.sensor), calibration);
}
