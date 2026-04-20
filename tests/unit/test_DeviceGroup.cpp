// Unit tests for eduart::sensorring::device::DeviceGroup (group of BaseDevice pointers, iteration, filtering by type).

#include <catch2/catch_all.hpp>
#include <functional>
#include <sensorring_transport/Protocol.hpp>
#include <string>
#include <vector>

#include "interface/ComInterface.hpp"
using namespace eduart::sensorring::transport::protocol;
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/DeviceGroup.hpp"
#include "sensorring/interface/ComEndpoint.hpp"

using eduart::sensorring::com::ComEndpoint;
using eduart::sensorring::com::ComInterface;
using eduart::sensorring::com::Direction;
using eduart::sensorring::device::BaseDevice;
using eduart::sensorring::device::DeviceGroup;
using eduart::sensorring::device::DeviceID;
using eduart::sensorring::device::DeviceType;
using eduart::sensorring::device::IDevice;

// Minimal ComInterface implementation for unit tests (no I/O).
class MockComInterface : public ComInterface {
public:
  MockComInterface()
      : ComInterface(eduart::sensorring::com::ComInterfaceID{}) {}

  bool send(ComEndpoint, std::uint8_t, const std::vector<std::uint8_t>&) override { return true; }
  bool openInterface() override { return true; }
  bool closeInterface() override { return true; }
  bool repairInterface() override { return true; }

protected:
  bool listener() override { return false; }
};

// Minimal BaseDevice-derived type for tests (implements BaseSensor pure virtuals).
class TestDeviceA : public BaseDevice {
public:
  TestDeviceA(ComInterface* iface, unsigned int idx = 0)
      : BaseDevice(DeviceID{ DeviceType::VL53L8CX, idx }, iface, ComEndpoint{ Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::VL53L8CX }, false) {}

  void onResetSensorState() override {}
  void onClearDataFlag() override {}
  void comCallback(ComEndpoint, std::uint8_t, const std::vector<std::uint8_t>&) override {}
};

// Second derived type for getDevicesOfType / createFromDevicesOfType tests.
class TestDeviceB : public BaseDevice {
public:
  TestDeviceB(ComInterface* iface, unsigned int idx = 0)
      : BaseDevice(DeviceID{ DeviceType::HTPA32, idx }, iface, ComEndpoint{ Direction::Output, static_cast<std::uint8_t>(idx + 1), devbyte::HTPA32 }, false) {}

  void onResetSensorState() override {}
  void onClearDataFlag() override {}
  void comCallback(ComEndpoint, std::uint8_t, const std::vector<std::uint8_t>&) override {}
};

TEST_CASE("DeviceGroup construction and getDevices", "[DeviceGroup]") {
  MockComInterface mock_com;
  TestDeviceA dev_a(&mock_com, 0);
  TestDeviceA dev_a1(&mock_com, 1);

  SECTION("empty group") {
    DeviceGroup group(std::vector<IDevice*>{});
    auto devices = group.getDevices();
    REQUIRE(devices.empty());
  }

  SECTION("single device") {
    std::vector<IDevice*> raw = { &dev_a };
    DeviceGroup group(raw);
    auto devices = group.getDevices();
    REQUIRE(devices.size() == 1u);
    REQUIRE(devices[0] == &dev_a);
  }

  SECTION("multiple devices") {
    std::vector<IDevice*> raw = { &dev_a, &dev_a1 };
    DeviceGroup group(raw);
    auto devices = group.getDevices();
    REQUIRE(devices.size() == 2u);
    REQUIRE(devices[0] == &dev_a);
    REQUIRE(devices[1] == &dev_a1);
  }
}

TEST_CASE("DeviceGroup invokeForEachDevice", "[DeviceGroup]") {
  MockComInterface mock_com;
  TestDeviceA dev_a(&mock_com, 0);
  TestDeviceA dev_a1(&mock_com, 1);

  SECTION("empty group") {
    DeviceGroup group(std::vector<IDevice*>{});
    int count = 0;
    group.invokeForEachDevice([&count](IDevice*) {
      ++count;
    });
    REQUIRE(count == 0);
  }

  SECTION("callback invoked for each device") {
    std::vector<IDevice*> raw = { &dev_a, &dev_a1 };
    DeviceGroup group(raw);
    int count = 0;
    std::vector<IDevice*> seen;
    group.invokeForEachDevice([&count, &seen](IDevice* d) {
      ++count;
      seen.push_back(d);
    });
    REQUIRE(count == 2);
    REQUIRE(seen.size() == 2u);
    REQUIRE(seen[0] == &dev_a);
    REQUIRE(seen[1] == &dev_a1);
  }
}

TEST_CASE("DeviceGroup getDevicesOfType", "[DeviceGroup]") {
  MockComInterface mock_com;
  TestDeviceA dev_a(&mock_com, 0);
  TestDeviceA dev_a1(&mock_com, 1);
  TestDeviceB dev_b(&mock_com, 0);

  SECTION("empty group returns empty vector") {
    DeviceGroup group(std::vector<IDevice*>{});
    auto of_a = group.getDevicesOfType<TestDeviceA>();
    auto of_b = group.getDevicesOfType<TestDeviceB>();
    REQUIRE(of_a.empty());
    REQUIRE(of_b.empty());
  }

  SECTION("only matching type returned") {
    std::vector<IDevice*> raw = { &dev_a, &dev_a1 };
    DeviceGroup group(raw);
    auto of_a = group.getDevicesOfType<TestDeviceA>();
    auto of_b = group.getDevicesOfType<TestDeviceB>();
    REQUIRE(of_a.size() == 2u);
    REQUIRE(of_b.empty());
    REQUIRE(of_a[0] == &dev_a);
    REQUIRE(of_a[1] == &dev_a1);
  }

  SECTION("mixed types filtered correctly") {
    std::vector<IDevice*> raw = { &dev_a, &dev_b, &dev_a1 };
    DeviceGroup group(raw);
    auto of_a = group.getDevicesOfType<TestDeviceA>();
    auto of_b = group.getDevicesOfType<TestDeviceB>();
    REQUIRE(of_a.size() == 2u);
    REQUIRE(of_b.size() == 1u);
    REQUIRE(of_a[0] == &dev_a);
    REQUIRE(of_a[1] == &dev_a1);
    REQUIRE(of_b[0] == &dev_b);
  }

  SECTION("BaseDevice returns all devices") {
    std::vector<IDevice*> raw = { &dev_a, &dev_b };
    DeviceGroup group(raw);
    auto all = group.getDevicesOfType<BaseDevice>();
    REQUIRE(all.size() == 2u);
    REQUIRE(all[0] == &dev_a);
    REQUIRE(all[1] == &dev_b);
  }
}

TEST_CASE("DeviceGroup invokeForEachDeviceOfType", "[DeviceGroup]") {
  MockComInterface mock_com;
  TestDeviceA dev_a(&mock_com, 0);
  TestDeviceB dev_b(&mock_com, 0);

  SECTION("empty group") {
    DeviceGroup group(std::vector<IDevice*>{});
    int count = 0;
    group.invokeForEachDeviceOfType<TestDeviceA>([&count](TestDeviceA*) {
      ++count;
    });
    REQUIRE(count == 0);
  }

  SECTION("callback only for matching type") {
    std::vector<IDevice*> raw = { &dev_a, &dev_b };
    DeviceGroup group(raw);
    int count_a = 0;
    int count_b = 0;
    group.invokeForEachDeviceOfType<TestDeviceA>([&count_a](TestDeviceA*) {
      ++count_a;
    });
    group.invokeForEachDeviceOfType<TestDeviceB>([&count_b](TestDeviceB*) {
      ++count_b;
    });
    REQUIRE(count_a == 1);
    REQUIRE(count_b == 1);
  }

  SECTION("callback receives correct pointer") {
    std::vector<IDevice*> raw = { &dev_a };
    DeviceGroup group(raw);
    TestDeviceA* received = nullptr;
    group.invokeForEachDeviceOfType<TestDeviceA>([&received](TestDeviceA* d) {
      received = d;
    });
    REQUIRE(received == &dev_a);
  }
}

TEST_CASE("DeviceGroup createFromDevicesOfType", "[DeviceGroup]") {
  MockComInterface mock_com;
  TestDeviceA dev_a(&mock_com, 0);
  TestDeviceA dev_a1(&mock_com, 1);
  TestDeviceB dev_b(&mock_com, 0);

  SECTION("empty input") {
    auto group = DeviceGroup::createFromDevicesOfType<TestDeviceA>(std::vector<IDevice*>{});
    REQUIRE(group.getDevices().empty());
  }

  SECTION("filters to requested type only") {
    std::vector<IDevice*> raw = { &dev_a, &dev_b, &dev_a1 };
    auto group                = DeviceGroup::createFromDevicesOfType<TestDeviceA>(raw);
    auto devices              = group.getDevices();
    REQUIRE(devices.size() == 2u);
    REQUIRE(devices[0] == &dev_a);
    REQUIRE(devices[1] == &dev_a1);
  }

  SECTION("createFromDevicesOfType TestDeviceB") {
    std::vector<IDevice*> raw = { &dev_a, &dev_b };
    auto group                = DeviceGroup::createFromDevicesOfType<TestDeviceB>(raw);
    auto devices              = group.getDevices();
    REQUIRE(devices.size() == 1u);
    REQUIRE(devices[0] == &dev_b);
  }

  SECTION("result group getDevicesOfType matches") {
    std::vector<IDevice*> raw = { &dev_a, &dev_b };
    auto group                = DeviceGroup::createFromDevicesOfType<TestDeviceA>(raw);
    auto of_a                 = group.getDevicesOfType<TestDeviceA>();
    REQUIRE(of_a.size() == 1u);
    REQUIRE(of_a[0] == &dev_a);
  }
}

TEST_CASE("DeviceGroup edge cases", "[DeviceGroup]") {
  MockComInterface mock_com;
  TestDeviceA dev(&mock_com, 0);

  SECTION("invokeForEachDevice with null callback not dereferenced for empty group") {
    DeviceGroup group(std::vector<IDevice*>{});
    group.invokeForEachDevice([](IDevice*) {});
  }

  SECTION("single device invokeForEachDeviceOfType") {
    DeviceGroup group(std::vector<IDevice*>{ &dev });
    int calls = 0;
    group.invokeForEachDeviceOfType<TestDeviceA>([&calls](TestDeviceA*) {
      ++calls;
    });
    REQUIRE(calls == 1);
  }
}

TEST_CASE("DeviceGroup waitForAll", "[DeviceGroup]") {
  using namespace std::chrono_literals;

  SECTION("empty futures vector returns true immediately") {
    std::vector<std::future<int> > futures;
    const auto timeout = 10ms;
    const auto ok      = DeviceGroup::waitForAll(futures, timeout, [](const int&) {
      return true;
    });
    REQUIRE(ok);
  }

  SECTION("all futures ready and predicate returns true for all") {
    std::promise<int> p1;
    std::promise<int> p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();

    // Make both futures ready before calling waitForAll.
    p1.set_value(1);
    p2.set_value(2);

    std::vector<std::future<int> > futures;
    futures.push_back(std::move(f1));
    futures.push_back(std::move(f2));

    const auto timeout  = 100ms;
    int predicate_calls = 0;
    const auto ok       = DeviceGroup::waitForAll(futures, timeout, [&predicate_calls](const int& value) {
      ++predicate_calls;
      return value > 0;
    });

    REQUIRE(ok);
    REQUIRE(predicate_calls == 2);
  }

  SECTION("all futures ready but predicate returns false for at least one") {
    std::promise<int> p1;
    std::promise<int> p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();

    p1.set_value(1);
    p2.set_value(0); // Will cause predicate to fail.

    std::vector<std::future<int> > futures;
    futures.push_back(std::move(f1));
    futures.push_back(std::move(f2));

    const auto timeout  = 100ms;
    int predicate_calls = 0;
    const auto ok       = DeviceGroup::waitForAll(futures, timeout, [&predicate_calls](const int& value) {
      ++predicate_calls;
      return value > 0;
    });

    REQUIRE_FALSE(ok);
    // Predicate should have been evaluated for all futures.
    REQUIRE(predicate_calls == 2);
  }

  SECTION("returns false if at least one future does not become ready before timeout") {
    std::promise<int> ready_promise;
    std::promise<int> never_ready_promise;

    auto ready_future       = ready_promise.get_future();
    auto never_ready_future = never_ready_promise.get_future();

    // Only fulfill one promise; the other future will time out.
    ready_promise.set_value(42);

    std::vector<std::future<int> > futures;
    futures.push_back(std::move(ready_future));
    futures.push_back(std::move(never_ready_future));

    const auto timeout  = 10ms;
    int predicate_calls = 0;
    const auto ok       = DeviceGroup::waitForAll(futures, timeout, [&predicate_calls](const int&) {
      ++predicate_calls;
      return true;
    });

    REQUIRE_FALSE(ok);
    // Predicate should not be called because waitForAll returns early on timeout.
    REQUIRE(predicate_calls == 0);
  }
}
