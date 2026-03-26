// Unit tests for SubscriberToken (unique subscription identifier).

#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "sensorring/types/SubscriberToken.hpp"

using eduart::subscription::SubscriberToken;

TEST_CASE("SubscriberToken construction and validity", "[SubscriberToken]") {
  SECTION("default constructed token is invalid") {
    SubscriberToken t;
    REQUIRE_FALSE(t.isValid());
    REQUIRE(t.value() == 0u);
  }

  SECTION("getNextToken returns valid tokens") {
    auto t1 = SubscriberToken::getNextToken();
    auto t2 = SubscriberToken::getNextToken();
    REQUIRE(t1.isValid());
    REQUIRE(t2.isValid());
    REQUIRE(t1.value() != 0u);
    REQUIRE(t2.value() != 0u);
    REQUIRE(t1.value() != t2.value());
  }

  SECTION("first token value is 1 so default token 0 stays invalid") {
    // Reset is not possible; we only check that valid tokens never have value 0.
    auto t = SubscriberToken::getNextToken();
    REQUIRE(t.value() != 0u);
    REQUIRE(t.isValid());
  }
}

TEST_CASE("SubscriberToken equality and inequality", "[SubscriberToken]") {
  SECTION("default tokens are equal") {
    SubscriberToken a, b;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
  }

  SECTION("different getNextToken values are not equal") {
    auto t1 = SubscriberToken::getNextToken();
    auto t2 = SubscriberToken::getNextToken();
    REQUIRE(t1 != t2);
    REQUIRE_FALSE(t1 == t2);
  }

  SECTION("same token copy is equal") {
    auto t1 = SubscriberToken::getNextToken();
    SubscriberToken t2 = t1;
    REQUIRE(t1 == t2);
    REQUIRE_FALSE(t1 != t2);
  }

  SECTION("default token is not equal to any getNextToken token") {
    SubscriberToken invalid;
    auto valid = SubscriberToken::getNextToken();
    REQUIRE(invalid != valid);
    REQUIRE_FALSE(invalid == valid);
  }
}

TEST_CASE("SubscriberToken value()", "[SubscriberToken]") {
  SECTION("default token value is 0") {
    SubscriberToken t;
    REQUIRE(t.value() == 0u);
  }

  SECTION("getNextToken returns strictly increasing values") {
    auto t1 = SubscriberToken::getNextToken();
    auto t2 = SubscriberToken::getNextToken();
    auto t3 = SubscriberToken::getNextToken();
    REQUIRE(t1.value() < t2.value());
    REQUIRE(t2.value() < t3.value());
  }
}

TEST_CASE("SubscriberToken in unordered containers", "[SubscriberToken]") {
  SECTION("tokens can be used as unordered_set keys") {
    std::unordered_set<SubscriberToken> set;
    REQUIRE(set.empty());

    auto t1 = SubscriberToken::getNextToken();
    auto t2 = SubscriberToken::getNextToken();
    set.insert(t1);
    set.insert(t2);
    REQUIRE(set.size() == 2u);
    REQUIRE(set.count(t1) == 1u);
    REQUIRE(set.count(t2) == 1u);

    set.erase(t1);
    REQUIRE(set.size() == 1u);
    REQUIRE(set.count(t1) == 0u);
    REQUIRE(set.count(t2) == 1u);
  }

  SECTION("default token can be stored and found in unordered_set") {
    std::unordered_set<SubscriberToken> set;
    SubscriberToken invalid;
    set.insert(invalid);
    REQUIRE(set.size() == 1u);
    REQUIRE(set.count(invalid) == 1u);

    SubscriberToken another_invalid;
    set.insert(another_invalid);
    REQUIRE(set.size() == 1u);
  }
}

TEST_CASE("SubscriberToken copy and move", "[SubscriberToken]") {
  SECTION("copy preserves value and validity") {
    auto orig = SubscriberToken::getNextToken();
    SubscriberToken copy = orig;
    REQUIRE(copy.value() == orig.value());
    REQUIRE(copy.isValid() == orig.isValid());
    REQUIRE(copy == orig);
  }

  SECTION("moved token preserves value") {
    auto orig = SubscriberToken::getNextToken();
    auto value_before = orig.value();
    SubscriberToken moved = std::move(orig);
    REQUIRE(moved.value() == value_before);
    REQUIRE(moved.isValid());
  }
}
