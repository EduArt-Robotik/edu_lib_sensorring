#include <catch2/catch_all.hpp>
#include <cstdint>
#include <numeric>
#include <vector>

#include "sensorring_transport/MessageAssembler.hpp"
#include "sensorring_transport/Protocol.hpp"
using namespace eduart::transport::protocol;

using eduart::sensorring::transport::Direction;
using eduart::sensorring::transport::MessageAssembler;
using eduart::sensorring::transport::TransportFrame;

namespace {

std::vector<std::uint8_t> makeData(std::size_t n, std::uint8_t start = 0) {
  std::vector<std::uint8_t> d(n);
  for (std::size_t i = 0; i < n; ++i)
    d[i] = static_cast<std::uint8_t>((start + i) & 0xFF);
  return d;
}

} // namespace

TEST_CASE("Assembly: empty payload produces single frame", "[Assembly]") {
  MessageAssembler asm_(61);
  auto frames = asm_.assemble(Direction::Output, 0x01, devbyte::BOARD, 0x03, {});

  REQUIRE(frames.size() == 1);
  REQUIRE(frames[0].fragmentType == fragment::SINGLE);
  REQUIRE(frames[0].fragmentCount == 0);
  REQUIRE(frames[0].data.empty());
  REQUIRE(frames[0].direction == Direction::Output);
  REQUIRE(frames[0].boardAddress == 0x01);
  REQUIRE(frames[0].deviceId == devbyte::BOARD);
  REQUIRE(frames[0].command == 0x03);
}

TEST_CASE("Assembly: small payload fits single frame", "[Assembly]") {
  MessageAssembler asm_(61);
  auto data   = makeData(30);
  auto frames = asm_.assemble(Direction::Input, 0x05, devbyte::VL53L8CX, 0x01, data);

  REQUIRE(frames.size() == 1);
  REQUIRE(frames[0].fragmentType == fragment::SINGLE);
  REQUIRE(frames[0].fragmentCount == 30);
  REQUIRE(frames[0].data == data);
}

TEST_CASE("Assembly: exactly max payload fits single frame", "[Assembly]") {
  MessageAssembler asm_(61);
  auto data   = makeData(61);
  auto frames = asm_.assemble(Direction::Output, 0x00, devbyte::HTPA32, 0x01, data);

  REQUIRE(frames.size() == 1);
  REQUIRE(frames[0].fragmentType == fragment::SINGLE);
  REQUIRE(frames[0].fragmentCount == 61);
  REQUIRE(frames[0].data == data);
}

TEST_CASE("Assembly: max+1 produces two fragments", "[Assembly]") {
  MessageAssembler asm_(61);
  auto data   = makeData(62);
  auto frames = asm_.assemble(Direction::Output, 0x01, devbyte::VL53L8CX, 0x04, data);

  REQUIRE(frames.size() == 2);

  // First frame
  REQUIRE(frames[0].fragmentType == fragment::FIRST);
  REQUIRE(frames[0].fragmentCount == 2); // total fragments
  REQUIRE(frames[0].data.size() == 61);
  REQUIRE(frames[0].data == makeData(61, 0));

  // Last frame
  REQUIRE(frames[1].fragmentType == fragment::LAST);
  REQUIRE(frames[1].fragmentCount == 1); // valid bytes in last
  REQUIRE(frames[1].data.size() == 1);
  REQUIRE(frames[1].data[0] == 61);
}

TEST_CASE("Assembly: three fragments with middle", "[Assembly]") {
  MessageAssembler asm_(61);
  auto data   = makeData(142); // 61 + 61 + 20
  auto frames = asm_.assemble(Direction::Input, 0x02, devbyte::VL53L8CX, 0x00, data);

  REQUIRE(frames.size() == 3);

  REQUIRE(frames[0].fragmentType == fragment::FIRST);
  REQUIRE(frames[0].fragmentCount == 3);
  REQUIRE(frames[0].data.size() == 61);

  REQUIRE(frames[1].fragmentType == fragment::MIDDLE);
  REQUIRE(frames[1].fragmentCount == 0);
  REQUIRE(frames[1].data.size() == 61);

  REQUIRE(frames[2].fragmentType == fragment::LAST);
  REQUIRE(frames[2].fragmentCount == 20);
  REQUIRE(frames[2].data.size() == 20);

  // Verify data integrity: concatenating all frame data should equal original
  std::vector<std::uint8_t> reassembled;
  for (const auto& f : frames) {
    reassembled.insert(reassembled.end(), f.data.begin(), f.data.end());
  }
  REQUIRE(reassembled == data);
}

TEST_CASE("Assembly: all frames carry correct routing info", "[Assembly]") {
  MessageAssembler asm_(61);
  auto data   = makeData(130);
  auto frames = asm_.assemble(Direction::Output, 0x0A, devbyte::WS2812B, 0x02, data);

  for (const auto& f : frames) {
    REQUIRE(f.direction == Direction::Output);
    REQUIRE(f.boardAddress == 0x0A);
    REQUIRE(f.deviceId == devbyte::WS2812B);
    REQUIRE(f.command == 0x02);
  }
}

TEST_CASE("Assembly: different maxPayloadPerFrame", "[Assembly]") {
  MessageAssembler asm_(20);
  auto data   = makeData(50);
  auto frames = asm_.assemble(Direction::Input, 0x01, devbyte::VL53L8CX, 0x01, data);

  REQUIRE(frames.size() == 3); // 20 + 20 + 10

  REQUIRE(frames[0].fragmentType == fragment::FIRST);
  REQUIRE(frames[0].fragmentCount == 3);
  REQUIRE(frames[0].data.size() == 20);

  REQUIRE(frames[1].fragmentType == fragment::MIDDLE);
  REQUIRE(frames[1].data.size() == 20);

  REQUIRE(frames[2].fragmentType == fragment::LAST);
  REQUIRE(frames[2].fragmentCount == 10);
  REQUIRE(frames[2].data.size() == 10);
}
