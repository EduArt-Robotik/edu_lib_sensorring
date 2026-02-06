// Unit tests for eduart::device::ICapability.

#include <catch2/catch_all.hpp>
#include <stdexcept>

#include "sensorring/device/ICapability.hpp"

using eduart::device::ICapability;

// Capability type for tests
struct SyncCap {
  struct Request {
    int value;
  };
  struct Response {
    int result;
  };
};

// Implements only const invoke (non-const delegates via base)
struct ConstOnlySyncImpl : ICapability<SyncCap> {
  SyncCap::Response invoke(const SyncCap::Request& req) const override {
    return SyncCap::Response{req.value * 2};
  }
};

// Implements both overloads
struct BothSyncImpl : ICapability<SyncCap> {
  SyncCap::Response invoke(const SyncCap::Request& req) override {
    return SyncCap::Response{req.value + 100};
  }
  SyncCap::Response invoke(const SyncCap::Request& req) const override {
    return SyncCap::Response{req.value + 200};
  }
};

// Overrides nothing: both invoke overloads use base default (const throws, non-const delegates to const)
struct NoOverrideSyncImpl : ICapability<SyncCap> {};

// Implements only non-const (inherits default const that throws)
struct NonConstOnlySyncImpl : ICapability<SyncCap> {
  SyncCap::Response invoke(const SyncCap::Request& req) override {
    return SyncCap::Response{req.value * 3};
  }
};

TEST_CASE("ICapability type aliases", "[ICapability]") {
  SECTION("Request and Response match capability type") {
    REQUIRE(std::is_same_v<ICapability<SyncCap>::Request, SyncCap::Request>);
    REQUIRE(std::is_same_v<ICapability<SyncCap>::Response, SyncCap::Response>);
  }
}

TEST_CASE("ICapability default const invoke throws", "[ICapability]") {
  // Base default: const invoke throws when not overridden (use type that overrides nothing)
  NoOverrideSyncImpl impl;
  const auto& cimpl = impl;

  REQUIRE_THROWS_AS(cimpl.invoke(SyncCap::Request{5}), std::runtime_error);
  REQUIRE_THROWS_WITH(
      cimpl.invoke(SyncCap::Request{5}),
      "const invoke not implemented for capability");
}

TEST_CASE("ICapability default non-const invoke rethrows when const not implemented", "[ICapability]") {
  // Non-const delegates to const; when const throws, base catches and rethrows with distinct message
  NoOverrideSyncImpl impl;

  REQUIRE_THROWS_AS(impl.invoke(SyncCap::Request{5}), std::runtime_error);
  REQUIRE_THROWS_WITH(
      impl.invoke(SyncCap::Request{5}),
      "try to invoke capability, but const invoke is not implemented");
}

TEST_CASE("ICapability non-const only override called on non-const object", "[ICapability]") {
  NonConstOnlySyncImpl impl;
  auto resp = impl.invoke(SyncCap::Request{5});
  REQUIRE(resp.result == 15);
}

TEST_CASE("ICapability non-const delegates to const when only const overridden", "[ICapability]") {
  ConstOnlySyncImpl impl;
  SyncCap::Request req{7};

  SECTION("non-const invoke returns const implementation result") {
    auto resp = impl.invoke(req);
    REQUIRE(resp.result == 14);
  }

  SECTION("const invoke returns same result") {
    const auto& cimpl = impl;
    auto resp = cimpl.invoke(req);
    REQUIRE(resp.result == 14);
  }
}

TEST_CASE("ICapability both overloads overridden", "[ICapability]") {
  BothSyncImpl impl;
  const BothSyncImpl& cimpl = impl;
  SyncCap::Request req{10};

  SECTION("non-const invoke uses non-const override") {
    auto resp = impl.invoke(req);
    REQUIRE(resp.result == 110);
  }

  SECTION("const invoke uses const override") {
    auto resp = cimpl.invoke(req);
    REQUIRE(resp.result == 210);
  }
}

TEST_CASE("ICapability polymorphic destruction", "[ICapability]") {
  ICapability<SyncCap>* base = new ConstOnlySyncImpl;
  REQUIRE_NOTHROW(delete base);
}

TEST_CASE("ICapability edge cases", "[ICapability]") {
  ConstOnlySyncImpl impl;

  SECTION("zero request value") {
    auto resp = impl.invoke(SyncCap::Request{0});
    REQUIRE(resp.result == 0);
  }

  SECTION("negative request value") {
    auto resp = impl.invoke(SyncCap::Request{-3});
    REQUIRE(resp.result == -6);
  }
}
