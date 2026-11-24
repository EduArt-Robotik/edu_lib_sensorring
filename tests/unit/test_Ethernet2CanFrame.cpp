#include <catch2/catch_all.hpp>

#include "interface/can/ethernet/Ethernet2CanFrame.hpp"
#include "sensorring/interface/InterfaceParams.hpp"

using namespace eduart::sensorring::com::ethernet;

TEST_CASE("Ethernet CAN UDP datagram preserves bus, CAN ID, and frame data", "[Ethernet2Can]") {
  const std::vector<std::uint8_t> can_data{ 0x01, 0x02, 0xFE, 0xFF };
  const auto datagram = encodeDatagram(3, 0x1ABCDE, can_data);

  REQUIRE(datagram.size() == DATAGRAM_HEADER_SIZE + can_data.size());
  REQUIRE(datagram[0] == 3);
  REQUIRE(datagram[1] == 0x00);
  REQUIRE(datagram[2] == 0x1A);
  REQUIRE(datagram[3] == 0xBC);
  REQUIRE(datagram[4] == 0xDE);

  std::uint32_t can_id = 0;
  std::vector<std::uint8_t> decoded_data;
  REQUIRE(decodeDatagram(datagram.data(), datagram.size(), 3, can_id, decoded_data));
  REQUIRE(can_id == 0x1ABCDE);
  REQUIRE(decoded_data == can_data);
}

TEST_CASE("Ethernet CAN UDP datagram rejects malformed and misrouted frames", "[Ethernet2Can]") {
  const std::vector<std::uint8_t> short_datagram{ 1, 0, 0, 0, 0 };
  std::uint32_t can_id = 0;
  std::vector<std::uint8_t> can_data;

  REQUIRE_FALSE(decodeDatagram(short_datagram.data(), short_datagram.size(), 1, can_id, can_data));

  const auto valid_datagram = encodeDatagram(1, 0x600, { 0x00, 0x01, 0x02 });
  REQUIRE_FALSE(decodeDatagram(valid_datagram.data(), valid_datagram.size(), 2, can_id, can_data));

  std::vector<std::uint8_t> oversized(DATAGRAM_HEADER_SIZE + 65, 0);
  oversized[0] = 1;
  REQUIRE_FALSE(decodeDatagram(oversized.data(), oversized.size(), 1, can_id, can_data));
}

TEST_CASE("Ethernet CAN parameters identify each gateway bus independently", "[Ethernet2Can]") {
  const eduart::sensorring::com::Ethernet2CanParams bus_a{ "gateway.local", 5000, 0 };
  const eduart::sensorring::com::Ethernet2CanParams bus_b{ "gateway.local", 5000, 1 };

  REQUIRE(bus_a.interfaceName() != bus_b.interfaceName());
  REQUIRE(bus_a.interfaceName() == "gateway.local:5000/0:0");
}
