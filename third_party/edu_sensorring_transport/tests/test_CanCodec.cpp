#include <catch2/catch_all.hpp>
#include <cstdint>
#include <vector>

#include "sensorring_transport/Protocol.hpp"
#include "sensorring_transport/can/CanCodec.hpp"
using namespace eduart::transport::protocol;

using eduart::sensorring::transport::Direction;
using eduart::sensorring::transport::TransportFrame;
using eduart::sensorring::transport::can::CanCodec;
using eduart::sensorring::transport::can::CanFrame;

TEST_CASE("CanCodec: makeCanId broadcast", "[CanCodec]") {
  std::uint32_t input, output;
  CanCodec::makeCanIds(0x00, input, output);

  REQUIRE(input == 0x600);  // (0b110 << 8) | (0 << 7) | 0x00
  REQUIRE(output == 0x680); // (0b110 << 8) | (1 << 7) | 0x00
}

TEST_CASE("CanCodec: makeCanId board 1", "[CanCodec]") {
  std::uint32_t input, output;
  CanCodec::makeCanIds(0x01, input, output);

  REQUIRE(input == 0x601);
  REQUIRE(output == 0x681);
}

TEST_CASE("CanCodec: makeCanId board 126", "[CanCodec]") {
  std::uint32_t input, output;
  CanCodec::makeCanIds(0x7E, input, output);

  REQUIRE(input == 0x67E);
  REQUIRE(output == 0x6FE);
}

TEST_CASE("CanCodec: makeCanId single arg", "[CanCodec]") {
  REQUIRE(CanCodec::makeCanId(1, 0x00) == 0x680); // output broadcast
  REQUIRE(CanCodec::makeCanId(0, 0x01) == 0x601); // input board 1
}

TEST_CASE("CanCodec: extractBoardAddress round-trip", "[CanCodec]") {
  for (std::uint8_t addr = 0; addr <= 0x7E; ++addr) {
    auto id = CanCodec::makeCanId(0, addr);
    REQUIRE(CanCodec::extractBoardAddress(id) == addr);
  }
}

TEST_CASE("CanCodec: extractDirection round-trip", "[CanCodec]") {
  auto input_id  = CanCodec::makeCanId(0, 0x05);
  auto output_id = CanCodec::makeCanId(1, 0x05);

  REQUIRE(CanCodec::extractDirection(input_id) == 0);
  REQUIRE(CanCodec::extractDirection(output_id) == 1);
}

TEST_CASE("CanCodec: fragment byte construction and extraction", "[CanCodec]") {
  SECTION("single frame with 30 data bytes") {
    auto fb = CanCodec::makeFragmentByte(fragment::SINGLE, 30);
    REQUIRE(fb == 0xDE);
    REQUIRE(CanCodec::fragmentType(fb) == fragment::SINGLE);
    REQUIRE(CanCodec::fragmentCount(fb) == 30);
  }

  SECTION("first fragment with 4 total") {
    auto fb = CanCodec::makeFragmentByte(fragment::FIRST, 4);
    REQUIRE(fb == 0x44);
    REQUIRE(CanCodec::fragmentType(fb) == fragment::FIRST);
    REQUIRE(CanCodec::fragmentCount(fb) == 4);
  }

  SECTION("middle fragment") {
    auto fb = CanCodec::makeFragmentByte(fragment::MIDDLE, 0);
    REQUIRE(fb == 0x00);
    REQUIRE(CanCodec::fragmentType(fb) == fragment::MIDDLE);
    REQUIRE(CanCodec::fragmentCount(fb) == 0);
  }

  SECTION("last fragment with 9 valid bytes") {
    auto fb = CanCodec::makeFragmentByte(fragment::LAST, 9);
    REQUIRE(fb == 0x89);
    REQUIRE(CanCodec::fragmentType(fb) == fragment::LAST);
    REQUIRE(CanCodec::fragmentCount(fb) == 9);
  }
}

TEST_CASE("CanCodec: encode produces correct CAN frame", "[CanCodec]") {
  TransportFrame frame;
  frame.direction     = Direction::Output;
  frame.boardAddress  = 0x05;
  frame.deviceId      = devbyte::VL53L8CX;
  frame.command       = 0x03;
  frame.fragmentType  = fragment::SINGLE;
  frame.fragmentCount = 3;
  frame.data          = { 0xAA, 0xBB, 0xCC };

  auto canFrame = CanCodec::encode(frame);

  REQUIRE(canFrame.id == CanCodec::makeCanId(1, 0x05));
  REQUIRE(canFrame.data.size() == 6); // 3 header + 3 data
  REQUIRE(canFrame.data[0] == CanCodec::makeFragmentByte(fragment::SINGLE, 3));
  REQUIRE(canFrame.data[1] == devbyte::VL53L8CX);
  REQUIRE(canFrame.data[2] == 0x03);
  REQUIRE(canFrame.data[3] == 0xAA);
  REQUIRE(canFrame.data[4] == 0xBB);
  REQUIRE(canFrame.data[5] == 0xCC);
}

TEST_CASE("CanCodec: decode recovers TransportFrame", "[CanCodec]") {
  // Raw CAN data: [fragByte, deviceId, command, payload...]
  std::uint8_t raw[] = { CanCodec::makeFragmentByte(fragment::SINGLE, 2), devbyte::HTPA32, 0x01, 0xDE, 0xAD };

  auto canId = CanCodec::makeCanId(0, 0x03); // input, board 3

  auto frame = CanCodec::decode(canId, raw, sizeof(raw));

  REQUIRE(frame.direction == Direction::Input);
  REQUIRE(frame.boardAddress == 0x03);
  REQUIRE(frame.deviceId == devbyte::HTPA32);
  REQUIRE(frame.command == 0x01);
  REQUIRE(frame.fragmentType == fragment::SINGLE);
  REQUIRE(frame.fragmentCount == 2);
  REQUIRE(frame.data.size() == 2);
  REQUIRE(frame.data[0] == 0xDE);
  REQUIRE(frame.data[1] == 0xAD);
}

TEST_CASE("CanCodec: encode/decode round-trip", "[CanCodec]") {
  TransportFrame original;
  original.direction     = Direction::Input;
  original.boardAddress  = 6;
  original.deviceId      = devbyte::VL53L8CX;
  original.command       = 0x04;
  original.fragmentType  = fragment::FIRST;
  original.fragmentCount = 3;
  original.data          = { 1, 2, 3, 4, 5 };

  auto canFrame = CanCodec::encode(original);
  auto decoded  = CanCodec::decode(canFrame.id, canFrame.data.data(), canFrame.data.size());

  REQUIRE(decoded.direction == original.direction);
  REQUIRE(decoded.boardAddress == original.boardAddress);
  REQUIRE(decoded.deviceId == original.deviceId);
  REQUIRE(decoded.command == original.command);
  REQUIRE(decoded.fragmentType == original.fragmentType);
  REQUIRE(decoded.fragmentCount == original.fragmentCount);
  REQUIRE(decoded.data == original.data);
}
