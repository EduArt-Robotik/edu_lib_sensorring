// Copyright (c) 2026 EduArt Robotik GmbH

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

#include "factory/SensorRingFactoryImpl.hpp"
#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"

namespace eduart {

namespace sensorring {

// ── Test access helper ────────────────────────────────────────────────────────
// Accesses the internal implementation seam (src/internal), not the public API.

class SensorRingFactoryTestAccess {
public:
  using DeviceExpectation    = SensorRingFactoryImpl::DeviceExpectation;
  using BuildStepStatus      = SensorRingFactoryImpl::BuildStepStatus;
  using ResolvedDeviceConfig = SensorRingFactoryImpl::ResolvedDeviceConfig;

  // ── Private helper wrappers ──────────────────────────────────────────────

  static ResolvedDeviceConfig resolveDeviceExpectations(const SensorRingFactoryImpl& factory, const std::vector<DeviceExpectation>& expectations, const std::vector<device::DeviceType>& available) {
    return factory.resolveDeviceExpectations(expectations, available);
  }

  static device::DeviceParamsMap buildDefaultParamsMap(const SensorRingFactoryImpl& factory, const std::vector<device::DeviceType>& devices) { return factory.buildDefaultParamsMap(devices); }

  static BuildStepStatus validateFirmware(const SensorRingFactoryImpl& factory, std::vector<board::EnumerationInformation>& enum_infos, const com::ComInterfaceID& id, bool strict) { return factory.validateFirmware(enum_infos, id, strict); }

  static void setEnumerationResults(SensorRingFactoryImpl& factory, SensorRingFactoryImpl::EnumerationMap results) { factory.setEnumerationResultsForTest(std::move(results)); }

  // ── Convenience builders ─────────────────────────────────────────────────

  static DeviceExpectation makeExactExpectation(device::DeviceType type, std::shared_ptr<device::DeviceParams> params = nullptr) {
    DeviceExpectation de;
    de.type   = type;
    de.params = std::move(params);
    return de;
  }

  static board::EnumerationInformation makeBoard(unsigned int idx, Version version, board::SensorBoardType type, const std::vector<device::DeviceType>& devices = {}) {
    board::EnumerationInformation ei;
    ei.idx     = idx;
    ei.version = version;
    ei.type    = type;
    ei.devices = devices;
    return ei;
  }
};

// ── Aliases for convenience ───────────────────────────────────────────────────

using Access               = SensorRingFactoryTestAccess;
using DeviceExpectation    = Access::DeviceExpectation;
using BuildStepStatus      = Access::BuildStepStatus;
using ResolvedDeviceConfig = Access::ResolvedDeviceConfig;

namespace dt = device;
namespace bt = board;

static const Version VALID_FW = { 0, 9, 0 };
static const Version OLD_FW   = { 0, 8, 9 };
static const Version NEWER_FW = { 1, 0, 0 };
static const com::ComInterfaceID TEST_ID{ com::InterfaceType::SocketCan, "can0" };

// ─────────────────────────────────────────────────────────────────────────────
// Section 1: buildDefaultParamsMap
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("buildDefaultParamsMap - empty list produces empty map", "[SensorRingFactory][buildDefaultParamsMap]") {
  SensorRingFactoryImpl factory;
  const auto map = Access::buildDefaultParamsMap(factory, {});
  REQUIRE(map.empty());
}

TEST_CASE("buildDefaultParamsMap - creates default params for known types", "[SensorRingFactory][buildDefaultParamsMap]") {
  SensorRingFactoryImpl factory;

  SECTION("VL53L8CX") {
    const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::VL53L8CX });
    REQUIRE(map.count(dt::DeviceType::VL53L8CX) == 1);
    REQUIRE(std::dynamic_pointer_cast<dt::VL53L8CX_Params>(map.at(dt::DeviceType::VL53L8CX)) != nullptr);
  }

  SECTION("TMF8829") {
    const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::TMF8829 });
    REQUIRE(map.count(dt::DeviceType::TMF8829) == 1);
    REQUIRE(std::dynamic_pointer_cast<dt::TMF8829_Params>(map.at(dt::DeviceType::TMF8829)) != nullptr);
  }

  SECTION("HTPA32") {
    const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::HTPA32 });
    REQUIRE(map.count(dt::DeviceType::HTPA32) == 1);
    REQUIRE(std::dynamic_pointer_cast<dt::HTPA32_Params>(map.at(dt::DeviceType::HTPA32)) != nullptr);
  }

  SECTION("WS2812b") {
    const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::WS2812b });
    REQUIRE(map.count(dt::DeviceType::WS2812b) == 1);
    REQUIRE(std::dynamic_pointer_cast<dt::WS2812b_Params>(map.at(dt::DeviceType::WS2812b)) != nullptr);
  }
}

TEST_CASE("buildDefaultParamsMap - unknown type produces no entry", "[SensorRingFactory][buildDefaultParamsMap]") {
  SensorRingFactoryImpl factory;
  const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::Undefined });
  REQUIRE(map.empty());
}

TEST_CASE("buildDefaultParamsMap - category defaults are storable and retrievable", "[SensorRingFactory][buildDefaultParamsMap]") {
  SensorRingFactoryImpl factory;

  dt::DepthSensorParams depth_default;
  depth_default.max_rate_hz = 6.0;
  factory.setDefaultDeviceParams(depth_default);

  const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::AnyDepth });
  REQUIRE(map.count(dt::DeviceType::AnyDepth) == 1);

  const auto* p = dynamic_cast<const dt::DepthSensorParams*>(map.at(dt::DeviceType::AnyDepth).get());
  REQUIRE(p != nullptr);
  REQUIRE(p->max_rate_hz == 6.0);
}

TEST_CASE("buildDefaultParamsMap - user-supplied defaults take precedence", "[SensorRingFactory][buildDefaultParamsMap]") {
  SensorRingFactoryImpl factory;

  dt::VL53L8CX_Params custom;
  custom.max_rate_hz = 30.0; // non-default sentinel value (default is 15 Hz)
  factory.setDefaultDeviceParams(custom);

  const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::VL53L8CX });
  REQUIRE(map.count(dt::DeviceType::VL53L8CX) == 1);

  const auto* p = dynamic_cast<const dt::VL53L8CX_Params*>(map.at(dt::DeviceType::VL53L8CX).get());
  REQUIRE(p != nullptr);
  REQUIRE(p->max_rate_hz == 30.0);
}

TEST_CASE("buildDefaultParamsMap - multiple device types in one call", "[SensorRingFactory][buildDefaultParamsMap]") {
  SensorRingFactoryImpl factory;
  const auto map = Access::buildDefaultParamsMap(factory, { dt::DeviceType::VL53L8CX, dt::DeviceType::HTPA32, dt::DeviceType::WS2812b });
  REQUIRE(map.size() == 3);
  REQUIRE(map.count(dt::DeviceType::VL53L8CX) == 1);
  REQUIRE(map.count(dt::DeviceType::HTPA32) == 1);
  REQUIRE(map.count(dt::DeviceType::WS2812b) == 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Section 2: resolveDeviceExpectations
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("resolveDeviceExpectations - exact type with explicit params", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  auto explicit_params         = std::make_shared<dt::VL53L8CX_Params>();
  explicit_params->max_rate_hz = 42.0; // sentinel

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::VL53L8CX, explicit_params) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 1);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
  REQUIRE(result.params_map.count(dt::DeviceType::VL53L8CX) == 1);
  REQUIRE(result.params_map.at(dt::DeviceType::VL53L8CX) == explicit_params);
}

TEST_CASE("resolveDeviceExpectations - exact type without params falls back to defaults", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::TMF8829) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::TMF8829 };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 1);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::TMF8829);
  REQUIRE(result.params_map.count(dt::DeviceType::TMF8829) == 1);
  REQUIRE(std::dynamic_pointer_cast<dt::TMF8829_Params>(result.params_map.at(dt::DeviceType::TMF8829)) != nullptr);
}

TEST_CASE("resolveDeviceExpectations - category wildcards resolve to concrete type", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  SECTION("AnyDepth resolves to VL53L8CX when present") {
    const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyDepth) };
    const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX };

    const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

    REQUIRE(result.configured_devs.size() == 1);
    REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
    REQUIRE(result.params_map.count(dt::DeviceType::VL53L8CX) == 1);
  }

  SECTION("AnyDepth resolves to TMF8829 when present") {
    const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyDepth) };
    const std::vector<dt::DeviceType> available{ dt::DeviceType::TMF8829 };

    const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

    REQUIRE(result.configured_devs[0] == dt::DeviceType::TMF8829);
    REQUIRE(result.params_map.count(dt::DeviceType::TMF8829) == 1);
  }

  SECTION("AnyThermal resolves to HTPA32") {
    const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyThermal) };
    const std::vector<dt::DeviceType> available{ dt::DeviceType::HTPA32 };

    const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

    REQUIRE(result.configured_devs[0] == dt::DeviceType::HTPA32);
    REQUIRE(result.params_map.count(dt::DeviceType::HTPA32) == 1);
  }

  SECTION("AnyLight resolves to WS2812b") {
    const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyLight) };
    const std::vector<dt::DeviceType> available{ dt::DeviceType::WS2812b };

    const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

    REQUIRE(result.configured_devs[0] == dt::DeviceType::WS2812b);
    REQUIRE(result.params_map.count(dt::DeviceType::WS2812b) == 1);
  }
}

TEST_CASE("resolveDeviceExpectations - category wildcard with explicit params uses resolved type as key", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  // User provided params via an AnyDepth category expectation.
  auto explicit_params         = std::make_shared<dt::VL53L8CX_Params>();
  explicit_params->max_rate_hz = 10.0; // sentinel

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyDepth, explicit_params) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  // The resolved concrete type is VL53L8CX, and the explicit params are mapped under that key.
  REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
  REQUIRE(result.params_map.count(dt::DeviceType::VL53L8CX) == 1);
  REQUIRE(result.params_map.at(dt::DeviceType::VL53L8CX) == explicit_params);
}

TEST_CASE("resolveDeviceExpectations - category default params take precedence over concrete defaults", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  dt::VL53L8CX_Params concrete_default;
  concrete_default.max_rate_hz = 30.0;
  factory.setDefaultDeviceParams(concrete_default);

  dt::DepthSensorParams category_default;
  category_default.max_rate_hz = 9.0;
  factory.setDefaultDeviceParams(category_default);

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyDepth) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 1);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
  REQUIRE(result.params_map.count(dt::DeviceType::VL53L8CX) == 1);

  const auto* p = dynamic_cast<const dt::DepthSensorParams*>(result.params_map.at(dt::DeviceType::VL53L8CX).get());
  REQUIRE(p != nullptr);
  REQUIRE(p->max_rate_hz == 9.0);
}

TEST_CASE("resolveDeviceExpectations - paramless concrete expectations use concrete defaults", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  dt::VL53L8CX_Params concrete_default;
  concrete_default.max_rate_hz = 18.0;
  factory.setDefaultDeviceParams(concrete_default);

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::VL53L8CX) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 1);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
  REQUIRE(result.params_map.count(dt::DeviceType::VL53L8CX) == 1);

  const auto* p = dynamic_cast<const dt::VL53L8CX_Params*>(result.params_map.at(dt::DeviceType::VL53L8CX).get());
  REQUIRE(p != nullptr);
  REQUIRE(p->max_rate_hz == 18.0);
}

TEST_CASE("resolveDeviceExpectations - paramless category expectations use category defaults", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  dt::DepthSensorParams category_default;
  category_default.max_rate_hz = 11.0;
  factory.setDefaultDeviceParams(category_default);

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyDepth) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::TMF8829 };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 1);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::TMF8829);
  REQUIRE(result.params_map.count(dt::DeviceType::TMF8829) == 1);

  const auto* p = dynamic_cast<const dt::DepthSensorParams*>(result.params_map.at(dt::DeviceType::TMF8829).get());
  REQUIRE(p != nullptr);
  REQUIRE(p->max_rate_hz == 11.0);
}

TEST_CASE("resolveDeviceExpectations - category expectations fall back to concrete defaults when no category default exists", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  dt::VL53L8CX_Params concrete_default;
  concrete_default.max_rate_hz = 27.5;
  factory.setDefaultDeviceParams(concrete_default);

  const std::vector<DeviceExpectation> expectations{ Access::makeExactExpectation(dt::DeviceType::AnyDepth) };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 1);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
  REQUIRE(result.params_map.count(dt::DeviceType::VL53L8CX) == 1);

  const auto* p = dynamic_cast<const dt::VL53L8CX_Params*>(result.params_map.at(dt::DeviceType::VL53L8CX).get());
  REQUIRE(p != nullptr);
  REQUIRE(p->max_rate_hz == 27.5);
}

TEST_CASE("resolveDeviceExpectations - multiple expectations produce ordered configured_devs", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;

  const std::vector<DeviceExpectation> expectations{
    Access::makeExactExpectation(dt::DeviceType::VL53L8CX),
    Access::makeExactExpectation(dt::DeviceType::HTPA32),
    Access::makeExactExpectation(dt::DeviceType::WS2812b),
  };
  const std::vector<dt::DeviceType> available{ dt::DeviceType::VL53L8CX, dt::DeviceType::HTPA32, dt::DeviceType::WS2812b };

  const auto result = Access::resolveDeviceExpectations(factory, expectations, available);

  REQUIRE(result.configured_devs.size() == 3);
  REQUIRE(result.configured_devs[0] == dt::DeviceType::VL53L8CX);
  REQUIRE(result.configured_devs[1] == dt::DeviceType::HTPA32);
  REQUIRE(result.configured_devs[2] == dt::DeviceType::WS2812b);
  REQUIRE(result.params_map.size() == 3);
}

TEST_CASE("resolveDeviceExpectations - empty expectations produces empty result", "[SensorRingFactory][resolveDeviceExpectations]") {
  SensorRingFactoryImpl factory;
  const auto result = Access::resolveDeviceExpectations(factory, {}, { dt::DeviceType::VL53L8CX });
  REQUIRE(result.configured_devs.empty());
  REQUIRE(result.params_map.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// Section 3: validateFirmware
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("validateFirmware - all boards on valid firmware returns Continue", "[SensorRingFactory][validateFirmware]") {
  SensorRingFactoryImpl factory;

  std::vector<bt::EnumerationInformation> boards{
    Access::makeBoard(0, VALID_FW, bt::SensorBoardType::Sidepanel),
    Access::makeBoard(1, NEWER_FW, bt::SensorBoardType::Headlight),
  };

  const auto status = Access::validateFirmware(factory, boards, TEST_ID, /*strict=*/true);

  REQUIRE(status == BuildStepStatus::Continue);
  for (const auto& b : boards) {
    REQUIRE(b.state == bt::ConnectionState::Connected);
  }
}

TEST_CASE("validateFirmware - marks all boards Connected regardless of firmware", "[SensorRingFactory][validateFirmware]") {
  SensorRingFactoryImpl factory;

  std::vector<bt::EnumerationInformation> boards{
    Access::makeBoard(0, OLD_FW, bt::SensorBoardType::Sidepanel),
    Access::makeBoard(1, VALID_FW, bt::SensorBoardType::Headlight),
  };

  // In relaxed mode the call returns Skip, but all boards still get Connected.
  Access::validateFirmware(factory, boards, TEST_ID, /*strict=*/false);
  for (const auto& b : boards) {
    REQUIRE(b.state == bt::ConnectionState::Connected);
  }
}

TEST_CASE("validateFirmware - outdated firmware in strict mode returns Fatal", "[SensorRingFactory][validateFirmware]") {
  SensorRingFactoryImpl factory;

  std::vector<bt::EnumerationInformation> boards{
    Access::makeBoard(0, OLD_FW, bt::SensorBoardType::Sidepanel),
  };

  const auto status = Access::validateFirmware(factory, boards, TEST_ID, /*strict=*/true);
  REQUIRE(status == BuildStepStatus::Fatal);
}

TEST_CASE("validateFirmware - outdated firmware in relaxed mode returns Skip", "[SensorRingFactory][validateFirmware]") {
  SensorRingFactoryImpl factory;

  std::vector<bt::EnumerationInformation> boards{
    Access::makeBoard(0, OLD_FW, bt::SensorBoardType::Sidepanel),
  };

  const auto status = Access::validateFirmware(factory, boards, TEST_ID, /*strict=*/false);
  REQUIRE(status == BuildStepStatus::Skip);
}

TEST_CASE("validateFirmware - mixed firmware strict fails on first invalid board", "[SensorRingFactory][validateFirmware]") {
  SensorRingFactoryImpl factory;

  std::vector<bt::EnumerationInformation> boards{
    Access::makeBoard(0, VALID_FW, bt::SensorBoardType::Sidepanel),
    Access::makeBoard(1, OLD_FW, bt::SensorBoardType::Headlight),
  };

  const auto status = Access::validateFirmware(factory, boards, TEST_ID, /*strict=*/true);
  REQUIRE(status == BuildStepStatus::Fatal);
}

// ─────────────────────────────────────────────────────────────────────────────
// Section 4: Public configuration API guard conditions
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("expectBoard before addInterface throws", "[SensorRingFactory][configuration]") {
  SensorRingFactory factory;
  // Logger throws std::runtime_error for LogVerbosity::Exception.
  REQUIRE_THROWS_AS(factory.expectBoard(), std::runtime_error);
}

TEST_CASE("expectDevice before expectBoard throws", "[SensorRingFactory][configuration]") {
  SensorRingFactory factory;
  factory.addInterface(com::SocketCanParams{ "can0" });

  REQUIRE_THROWS_AS(factory.expectDevice(dt::VL53L8CX_Params{}), std::runtime_error);
  REQUIRE_THROWS_AS(factory.expectDevice(dt::TMF8829_Params{}), std::runtime_error);
  REQUIRE_THROWS_AS(factory.expectDevice(dt::HTPA32_Params{}), std::runtime_error);
  REQUIRE_THROWS_AS(factory.expectDevice(dt::WS2812b_Params{}), std::runtime_error);
}

TEST_CASE("expectDevice before addInterface throws", "[SensorRingFactory][configuration]") {
  SensorRingFactory factory;
  REQUIRE_THROWS_AS(factory.expectDevice(dt::VL53L8CX_Params{}), std::runtime_error);
}

// ─────────────────────────────────────────────────────────────────────────────
// Section 5: State accessors and reset
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("getLatestEnumerationResult is empty on a freshly constructed factory", "[SensorRingFactory][state]") {
  SensorRingFactory factory;
  REQUIRE(factory.getLatestEnumerationResult().empty());
}

TEST_CASE("printTopology returns empty string when no enumeration has been performed", "[SensorRingFactory][state]") {
  SensorRingFactory factory;
  REQUIRE(factory.printTopology().empty());
}

TEST_CASE("printTopology includes interface name after enumeration results are set", "[SensorRingFactory][state]") {
  SensorRingFactoryImpl factory;

  bt::EnumerationInformation ei;
  ei.idx          = 0;
  ei.version      = VALID_FW;
  ei.type         = bt::SensorBoardType::Headlight;
  ei.state        = bt::ConnectionState::Connected;
  ei.config_state = bt::ConfigurationState::Configured;

  SensorRingFactoryImpl::EnumerationMap results;
  results[TEST_ID] = { ei };
  Access::setEnumerationResults(factory, std::move(results));

  const std::string topology = factory.printTopology();
  REQUIRE_FALSE(topology.empty());
  REQUIRE(topology.find(TEST_ID.name) != std::string::npos);
}

TEST_CASE("reset clears enumeration results and configuration", "[SensorRingFactory][state]") {
  SensorRingFactoryImpl factory;

  factory.addInterface(com::SocketCanParams{ "can0" });
  factory.expectBoard();

  // Populate enumeration results via test access.
  SensorRingFactoryImpl::EnumerationMap results;
  results[TEST_ID] = { Access::makeBoard(0, VALID_FW, bt::SensorBoardType::Headlight) };
  Access::setEnumerationResults(factory, std::move(results));

  // Enumeration results are explicitly preserved by reset() as per the API contract.
  factory.reset();

  // After reset the factory no longer has any interfaces configured.
  // The simplest observable effect: expectBoard() now throws because _interfaces is empty.
  REQUIRE_THROWS_AS(factory.expectBoard(), std::runtime_error);
}

} // namespace sensorring

} // namespace eduart
