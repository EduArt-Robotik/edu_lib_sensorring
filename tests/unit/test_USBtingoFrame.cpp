#include <catch2/catch_all.hpp>
#include <sensorring_transport/MessageAssembler.hpp>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>
#include <usbtingo/can/Dlc.hpp>

#include "interface/can/usbtingo/USBtingoFrame.hpp"

using namespace eduart::sensorring;

TEST_CASE("USBtingo transmits CAN FD frames with DLC codes and zero padding", "[USBtingoFrame]") {
  const std::array<std::size_t, 16> dlc_lengths{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };
  for (std::size_t length = 0; length <= 64; ++length) {
    CAPTURE(length);
    std::vector<std::uint8_t> data(length);
    for (std::size_t i = 0; i < length; ++i) {
      data[i] = static_cast<std::uint8_t>(i + 1);
    }

    const auto frame = com::makeUsbTingoCanTxFrame(0x680, data, true);
    REQUIRE(frame.id == 0x680);
    REQUIRE(frame.fdf == 1);
    REQUIRE(frame.brs == 1);
    const auto expected = std::lower_bound(dlc_lengths.begin(), dlc_lengths.end(), length) - dlc_lengths.begin();
    REQUIRE(frame.dlc == expected);
    const auto padded_length = usbtingo::can::Dlc::dlc_to_bytes(frame.dlc);
    REQUIRE(padded_length >= length);
    REQUIRE(std::equal(data.begin(), data.end(), frame.data.begin()));
    REQUIRE(std::all_of(frame.data.begin() + length, frame.data.end(), [](std::uint8_t byte) {
      return byte == 0;
    }));

    std::array<std::uint8_t, 76> serialized{};
    REQUIRE(usbtingo::device::CanTxFrame::serialize_can_frame(serialized.data(), frame));
    REQUIRE((serialized[10] & 0x0f) == frame.dlc);
    REQUIRE(std::equal(frame.data.begin(), frame.data.begin() + padded_length, serialized.begin() + 12));
  }

  REQUIRE(com::makeUsbTingoCanTxFrame(0x600, std::vector<std::uint8_t>(11), false).dlc == 9);
  REQUIRE(com::makeUsbTingoCanTxFrame(0x600, std::vector<std::uint8_t>(59), false).dlc == 15);
  REQUIRE(com::makeUsbTingoCanTxFrame(0x600, {}, false).brs == 0);
  REQUIRE_THROWS_AS(com::makeUsbTingoCanTxFrame(0x600, std::vector<std::uint8_t>(65), false), std::invalid_argument);
}

TEST_CASE("USBtingo preserves single and batched bootloader request frames", "[USBtingoFrame]") {
  using namespace transport::protocol;
  transport::MessageAssembler assembler(transport::can::CanCodec::MAX_PAYLOAD_PER_FRAME);
  for (const std::size_t count : { 1U, 7U }) {
    CAPTURE(count);
    std::vector<std::uint8_t> payload(count * 8);
    for (std::size_t i = 0; i < payload.size(); ++i) {
      payload[i] = static_cast<std::uint8_t>(i + 1);
    }

    const auto fragments = assembler.assemble(transport::Direction::Input, 0, devbyte::BOARD, sensor_board::BOOTLOADER_REQUEST, payload);
    REQUIRE(fragments.size() == 1);
    const auto can_frame = transport::can::CanCodec::encode(fragments.front());
    const auto usb_frame = com::makeUsbTingoCanTxFrame(can_frame.id, can_frame.data, false);
    std::array<std::uint8_t, 76> serialized{};
    REQUIRE(usbtingo::device::CanTxFrame::serialize_can_frame(serialized.data(), usb_frame));
    const auto wire_dlc = serialized[10] & 0x0f;
    const auto decoded  = transport::can::CanCodec::decode(can_frame.id, serialized.data() + 12, usbtingo::can::Dlc::dlc_to_bytes(wire_dlc));

    REQUIRE(decoded.fragmentType == fragment::SINGLE);
    REQUIRE(decoded.fragmentCount == payload.size());
    REQUIRE(decoded.deviceId == devbyte::BOARD);
    REQUIRE(decoded.command == sensor_board::BOOTLOADER_REQUEST);
    REQUIRE(decoded.data.size() >= payload.size());
    REQUIRE(std::equal(payload.begin(), payload.end(), decoded.data.begin()));
  }
}
