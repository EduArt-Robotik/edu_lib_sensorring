// Unit tests for Subscription (RAII unsubscribe wrapper).

#include <catch2/catch_all.hpp>

#include "sensorring/subscription/Subscription.hpp"

using eduart::subscription::Subscription;
using eduart::subscription::SubscriberToken;

TEST_CASE("Subscription default construction", "[Subscription]") {
  Subscription sub;
  REQUIRE_FALSE(sub.isActive());
}

TEST_CASE("Subscription constructed with token and cancel callable", "[Subscription]") {
  bool cancelled = false;
  auto token     = SubscriberToken::getNextToken();

  Subscription sub(token, [&cancelled]() { cancelled = true; });
  REQUIRE(sub.isActive());
  REQUIRE(sub.token() == token);

  SECTION("cancel() invokes the unsubscribe callable") {
    sub.cancel();
    REQUIRE(cancelled);
    REQUIRE_FALSE(sub.isActive());
  }

  SECTION("cancel() is idempotent") {
    sub.cancel();
    REQUIRE(cancelled);
    cancelled = false;
    sub.cancel();
    REQUIRE_FALSE(cancelled);
  }
}

TEST_CASE("Subscription RAII: destructor auto-cancels", "[Subscription]") {
  bool cancelled = false;
  {
    Subscription sub(SubscriberToken::getNextToken(), [&cancelled]() { cancelled = true; });
    REQUIRE(sub.isActive());
  }
  REQUIRE(cancelled);
}

TEST_CASE("Subscription is move-only", "[Subscription]") {
  bool cancelled = false;
  auto token     = SubscriberToken::getNextToken();

  Subscription original(token, [&cancelled]() { cancelled = true; });

  SECTION("move constructor transfers ownership") {
    Subscription moved(std::move(original));
    REQUIRE(moved.isActive());
    REQUIRE(moved.token() == token);
    REQUIRE_FALSE(original.isActive());
  }

  SECTION("move assignment transfers ownership and cancels previous") {
    bool prev_cancelled = false;
    Subscription prev(SubscriberToken::getNextToken(), [&prev_cancelled]() { prev_cancelled = true; });

    prev = std::move(original);
    REQUIRE(prev_cancelled);
    REQUIRE(prev.isActive());
    REQUIRE(prev.token() == token);
    REQUIRE_FALSE(original.isActive());
  }

  SECTION("self-move-assignment is safe") {
    auto* ptr = &original;
    original  = std::move(*ptr);
    REQUIRE(original.isActive());
    REQUIRE_FALSE(cancelled);
  }
}
