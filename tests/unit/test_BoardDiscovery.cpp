#include <catch2/catch_all.hpp>
#include <sensorring_transport/Protocol.hpp>
#include <stdexcept>

#include "board/BoardEnumerator.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/board/BoardDiscovery.hpp"

using namespace eduart::sensorring;
using namespace eduart::sensorring::transport::protocol;

namespace {

class EnumerationInterface : public com::ComInterface {
public:
  EnumerationInterface()
      : ComInterface({ com::InterfaceType::SocketCan, "test" }) {}

  bool send(com::ComEndpoint target, std::uint8_t command, const std::vector<std::uint8_t>& data) override {
    REQUIRE(target == com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::BOARD });
    REQUIRE(command == sensor_board::ACTIVE_DEVICE_REQUEST);
    REQUIRE(data.empty());
    ++requests;
    return send_success;
  }

  bool openInterface() override { return true; }
  bool closeInterface() override { return true; }
  bool repairInterface() override { return true; }

  void receive(std::uint8_t command, const std::vector<std::uint8_t>& data) { dispatchMessage({ com::Direction::Output, 1, devbyte::BOARD }, command, data); }

  unsigned int requests = 0;
  bool send_success     = true;

protected:
  bool listener() override { return true; }
};

} // namespace

TEST_CASE("Board discovery starts with no interfaces or results", "[BoardDiscovery]") {
  board::BoardDiscovery discovery;
  REQUIRE(discovery.enumerate().empty());
  REQUIRE(discovery.getLatestEnumerationResult().empty());
  REQUIRE(discovery.printTopology().empty());
}

TEST_CASE("Board enumeration sends a broadcast and decodes responses", "[BoardDiscovery]") {
  EnumerationInterface interface;
  board::BoardEnumerator enumerator(&interface);
  enumerator.startEnumeration();
  REQUIRE(interface.requests == 1);
  REQUIRE(enumerator.getResult().empty());

  interface.receive(sensor_board::ACTIVE_DEVICE_RESPONSE, { 0, 0, 0, 9, 1, 0, 0, 0, 0, 2, 0 });
  const auto results = enumerator.getResult();
  REQUIRE(results.size() == 1);
  REQUIRE(results[0].idx == 0);
  REQUIRE(results[0].type == board::SensorBoardType::Sidepanel);
  REQUIRE(results[0].version == Version{ 0, 9, 1 });
  REQUIRE(results[0].state == board::ConnectionState::Connected);
  REQUIRE(results[0].config_state == board::ConfigurationState::Undefined);
  REQUIRE(results[0].hasDevice(device::DeviceType::VL53L8CX));
  REQUIRE(results[0].devices == std::vector<device::DeviceType>{ device::DeviceType::VL53L8CX });

  interface.receive(sensor_board::RESET, {});
  REQUIRE(enumerator.getResult().size() == 1);
}

TEST_CASE("Board discovery preserves topology formatting", "[BoardDiscovery]") {
  board::EnumerationInformation info;
  info.idx     = 2;
  info.type    = board::SensorBoardType::Sidepanel;
  info.state   = board::ConnectionState::Connected;
  info.version = { 0, 9, 1 };
  board::BoardDiscovery::EnumerationMap results;
  results[{ com::InterfaceType::SocketCan, "test" }] = { info };

  const auto expected = "\n\n=================================================\n"
                        "Topology of the sensors on test:\n\n"
                        + info.toString()
                        + "\n"
                          "=================================================\n";
  REQUIRE(board::BoardDiscovery::printTopology(results) == expected);
}

TEST_CASE("Board enumeration reports send failures", "[BoardDiscovery]") {
  EnumerationInterface interface;
  interface.send_success = false;
  board::BoardEnumerator enumerator(&interface);
  REQUIRE_THROWS_AS(enumerator.startEnumeration(), std::runtime_error);
}

TEST_CASE("Board enumeration unsubscribes when destroyed", "[BoardDiscovery]") {
  EnumerationInterface interface;
  {
    board::BoardEnumerator enumerator(&interface);
    enumerator.startEnumeration();
  }
  interface.receive(sensor_board::ACTIVE_DEVICE_RESPONSE, { 0, 0, 0, 9, 1, 0, 0, 0, 0, 2, 0 });
  board::BoardEnumerator enumerator(&interface);
  REQUIRE(enumerator.getResult().empty());
}
