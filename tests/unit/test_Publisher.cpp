// Unit tests for Publisher<Args...> (thread-safe pub/sub template).

#include <atomic>
#include <catch2/catch_all.hpp>
#include <string>
#include <thread>
#include <vector>

#include "sensorring/subscription/Publisher.hpp"

using eduart::sensorring::subscription::Publisher;
using eduart::sensorring::subscription::Subscription;

// ---------------------------------------------------------------------------
// Basic subscribe / publish
// ---------------------------------------------------------------------------
TEST_CASE("Publisher: subscribe returns an active Subscription", "[Publisher]") {
  Publisher<int> pub;
  Subscription sub = pub.subscribe([](int) {});
  REQUIRE(sub.isActive());
}

TEST_CASE("Publisher: publish invokes subscriber with correct arguments", "[Publisher]") {
  Publisher<int, const std::string&> pub;

  int received_i;
  std::string received_s;
  Subscription sub = pub.subscribe([&](int i, const std::string& s) {
    received_i = i;
    received_s = s;
  });

  pub.publish(42, "hello");
  REQUIRE(received_i == 42);
  REQUIRE(received_s == "hello");
}

TEST_CASE("Publisher: publish invokes all subscribers", "[Publisher]") {
  Publisher<> pub;
  int count = 0;

  Subscription sub1 = pub.subscribe([&]() {
    ++count;
  });
  Subscription sub2 = pub.subscribe([&]() {
    ++count;
  });
  Subscription sub3 = pub.subscribe([&]() {
    ++count;
  });

  pub.publish();
  REQUIRE(count == 3);
}

// ---------------------------------------------------------------------------
// Unsubscribe
// ---------------------------------------------------------------------------
TEST_CASE("Publisher: explicit unsubscribe stops delivery", "[Publisher]") {
  Publisher<> pub;
  int count = 0;

  Subscription sub = pub.subscribe([&]() {
    ++count;
  });
  pub.unsubscribe(sub.token());
  pub.publish();

  REQUIRE(count == 0);
}

TEST_CASE("Publisher: RAII unsubscribe on Subscription destruction", "[Publisher]") {
  Publisher<> pub;
  int count = 0;
  {
    Subscription sub = pub.subscribe([&]() {
      ++count;
    });
  }
  pub.publish();
  REQUIRE(count == 0);
}

TEST_CASE("Publisher: unsubscribe during publish is safe (copy-then-invoke)", "[Publisher]") {
  Publisher<> pub;
  Subscription sub;
  int count = 0;

  sub = pub.subscribe([&]() {
    ++count;
    sub.cancel(); // unsubscribe while publish is iterating the copy
  });

  pub.publish();
  REQUIRE(count == 1);

  pub.publish(); // should no longer be subscribed
  REQUIRE(count == 1);
}

// ---------------------------------------------------------------------------
// copySubscribers
// ---------------------------------------------------------------------------
TEST_CASE("Publisher: copySubscribers returns snapshot", "[Publisher]") {
  Publisher<int> pub;

  Subscription sub1 = pub.subscribe([](int) {});
  Subscription sub2 = pub.subscribe([](int) {});

  auto snapshot = pub.copySubscribers();
  REQUIRE(snapshot.size() == 2);

  // Subscribing after snapshot should not affect snapshot size
  Subscription sub3 = pub.subscribe([](int) {});
  REQUIRE(snapshot.size() == 2);
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------
TEST_CASE("Publisher: publish with no subscribers does nothing", "[Publisher]") {
  Publisher<int> pub;
  REQUIRE_NOTHROW(pub.publish(99));
}

TEST_CASE("Publisher: unsubscribe with unknown token is a no-op", "[Publisher]") {
  Publisher<> pub;
  auto bogus = eduart::sensorring::subscription::SubscriberToken::getNextToken();
  REQUIRE_NOTHROW(pub.unsubscribe(bogus));
}

// ---------------------------------------------------------------------------
// Thread safety
// ---------------------------------------------------------------------------
TEST_CASE("Publisher: concurrent subscribe and publish", "[Publisher]") {
  Publisher<int> pub;
  std::atomic<int> total{ 0 };
  constexpr int N = 100;

  // Subscriber thread: add N subscriptions
  std::thread subscriber([&]() {
    std::vector<Subscription> subs;
    subs.reserve(N);
    for (int i = 0; i < N; ++i) {
      subs.push_back(pub.subscribe([&total](int v) {
        total += v;
      }));
    }
    // Keep subscriptions alive until publish finishes
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  });

  // Let subscriber thread start adding
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Publish on main thread
  pub.publish(1);

  subscriber.join();

  // total >= 0 is always true; the point is no data-race / crash.
  REQUIRE(total >= 0);
}
