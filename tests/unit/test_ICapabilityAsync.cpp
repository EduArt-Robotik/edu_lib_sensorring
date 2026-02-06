// Unit tests for eduart::device::ICapabilityAsync.

#include <catch2/catch_all.hpp>
#include <future>
#include <stdexcept>

#include "sensorring/device/ICapabilityAsync.hpp"

using eduart::device::ICapabilityAsync;

// Capability type for async tests
struct AsyncCap {
  struct Request {
    int value;
  };
  struct Response {
    int result;
  };
};

// Implements only const invoke_async (non-const delegates via base)
struct ConstOnlyAsyncImpl : ICapabilityAsync<AsyncCap> {
  std::future<AsyncCap::Response> invoke_async(const AsyncCap::Request& req) const override {
    return std::async(std::launch::deferred, [req]() {
      return AsyncCap::Response{req.value * 2};
    });
  }
};

// Implements both overloads
struct BothAsyncImpl : ICapabilityAsync<AsyncCap> {
  std::future<AsyncCap::Response> invoke_async(const AsyncCap::Request& req) override {
    return std::async(std::launch::deferred, [req]() {
      return AsyncCap::Response{req.value + 100};
    });
  }
  std::future<AsyncCap::Response> invoke_async(const AsyncCap::Request& req) const override {
    return std::async(std::launch::deferred, [req]() {
      return AsyncCap::Response{req.value + 200};
    });
  }
};

// Overrides nothing: both invoke_async overloads use base default (const throws, non-const delegates to const)
struct NoOverrideAsyncImpl : ICapabilityAsync<AsyncCap> {};

// Implements only non-const (inherits default const that throws)
struct NonConstOnlyAsyncImpl : ICapabilityAsync<AsyncCap> {
  std::future<AsyncCap::Response> invoke_async(const AsyncCap::Request& req) override {
    return std::async(std::launch::deferred, [req]() {
      return AsyncCap::Response{req.value * 3};
    });
  }
};

TEST_CASE("ICapabilityAsync type aliases", "[ICapabilityAsync]") {
  SECTION("Request and Response match capability type") {
    REQUIRE(std::is_same_v<ICapabilityAsync<AsyncCap>::Request, AsyncCap::Request>);
    REQUIRE(std::is_same_v<ICapabilityAsync<AsyncCap>::Response, AsyncCap::Response>);
  }
}

TEST_CASE("ICapabilityAsync default const invoke_async throws", "[ICapabilityAsync]") {
  // Base default: const invoke_async throws when not overridden (use type that overrides nothing)
  NoOverrideAsyncImpl impl;
  const auto& cimpl = impl;

  REQUIRE_THROWS_AS(cimpl.invoke_async(AsyncCap::Request{5}), std::runtime_error);
  REQUIRE_THROWS_WITH(
      cimpl.invoke_async(AsyncCap::Request{5}),
      "const async invoke not implemented for capability");
}

TEST_CASE("ICapabilityAsync default non-const invoke_async rethrows when const not implemented", "[ICapabilityAsync]") {
  // Non-const delegates to const; when const throws, base catches and rethrows with distinct message
  NoOverrideAsyncImpl impl;

  REQUIRE_THROWS_AS(impl.invoke_async(AsyncCap::Request{5}), std::runtime_error);
  REQUIRE_THROWS_WITH(
      impl.invoke_async(AsyncCap::Request{5}),
      "try to invoke capability asynchronously, but const invoke_async is not implemented");
}

TEST_CASE("ICapabilityAsync non-const only override called on non-const object", "[ICapabilityAsync]") {
  NonConstOnlyAsyncImpl impl;
  auto fut = impl.invoke_async(AsyncCap::Request{5});
  REQUIRE(fut.valid());
  auto resp = fut.get();
  REQUIRE(resp.result == 15);
}

TEST_CASE("ICapabilityAsync non-const delegates to const when only const overridden", "[ICapabilityAsync]") {
  ConstOnlyAsyncImpl impl;
  AsyncCap::Request req{7};

  SECTION("non-const invoke_async returns future with const implementation result") {
    auto fut = impl.invoke_async(req);
    REQUIRE(fut.valid());
    auto resp = fut.get();
    REQUIRE(resp.result == 14);
  }

  SECTION("const invoke_async returns same result") {
    const auto& cimpl = impl;
    auto fut = cimpl.invoke_async(req);
    REQUIRE(fut.valid());
    auto resp = fut.get();
    REQUIRE(resp.result == 14);
  }
}

TEST_CASE("ICapabilityAsync both overloads overridden", "[ICapabilityAsync]") {
  BothAsyncImpl impl;
  const BothAsyncImpl& cimpl = impl;
  AsyncCap::Request req{10};

  SECTION("non-const invoke_async uses non-const override") {
    auto fut = impl.invoke_async(req);
    REQUIRE(fut.valid());
    auto resp = fut.get();
    REQUIRE(resp.result == 110);
  }

  SECTION("const invoke_async uses const override") {
    auto fut = cimpl.invoke_async(req);
    REQUIRE(fut.valid());
    auto resp = fut.get();
    REQUIRE(resp.result == 210);
  }
}

TEST_CASE("ICapabilityAsync polymorphic destruction", "[ICapabilityAsync]") {
  ICapabilityAsync<AsyncCap>* base = new ConstOnlyAsyncImpl;
  REQUIRE_NOTHROW(delete base);
}

TEST_CASE("ICapabilityAsync edge cases", "[ICapabilityAsync]") {
  ConstOnlyAsyncImpl impl;

  SECTION("zero request value") {
    auto fut = impl.invoke_async(AsyncCap::Request{0});
    auto resp = fut.get();
    REQUIRE(resp.result == 0);
  }

  SECTION("negative request value") {
    auto fut = impl.invoke_async(AsyncCap::Request{-3});
    auto resp = fut.get();
    REQUIRE(resp.result == -6);
  }
}
