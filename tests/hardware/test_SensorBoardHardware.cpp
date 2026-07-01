#include <catch2/catch_test_macros.hpp>
#include <memory>

#define private public
#include "sensorring/board/SensorBoard.hpp"
#undef private

#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/interface/InterfaceParams.hpp"

using eduart::sensorring::SensorRing;
using eduart::sensorring::SensorRingFactory;
using eduart::sensorring::board::Orientation;
using eduart::sensorring::board::SensorBoard;
using eduart::sensorring::com::InterfaceType;
using eduart::sensorring::com::SocketCanParams;
using eduart::sensorring::com::UsbTingoParams;

namespace {

struct HardwareContext {
  std::unique_ptr<SensorRing> ring;
  SensorBoard* board = nullptr;
};

HardwareContext open_first_board(const std::string& interface_name, InterfaceType type) {
  HardwareContext ctx;

  try {
    SensorRingFactory factory;

    if (type == InterfaceType::SocketCan) {
      factory.addInterface(SocketCanParams{ interface_name });
    } else {
      factory.addInterface(UsbTingoParams{ interface_name });
    }
    factory.expectBoard({});

    ctx.ring = factory.build();
    if (!ctx.ring) {
      return ctx;
    }

    for (auto* bus : ctx.ring->getSensorBuses()) {
      const auto boards = bus->getSensorBoards();
      if (!boards.empty()) {
        ctx.board = boards.front();
        break;
      }
    }
  } catch (...) {
    ctx.ring.reset();
    ctx.board = nullptr;
  }

  return ctx;
}

} // namespace

// NOTE:
// These tests require a single sensor board connected via USBtingo or SocketCAN.
// Enable via the SENSORRING_BUILD_HARDWARE_TESTS CMake option.

TEST_CASE("Sensor board orientation parameter get/set round-trip", "[SensorBoardHardware]") {
  HardwareContext ctx = open_first_board("0", InterfaceType::UsbTingo);

  if (!ctx.board) {
    ctx = open_first_board("can0", InterfaceType::SocketCan);
  }

  if (!ctx.board) {
    FAIL("No sensor board found on USBtingo(0) or SocketCAN(can0). Ensure one board is connected.");
  }

  SensorBoard& board = *ctx.board;

  Orientation original = Orientation::None;
  REQUIRE(board.getOrientation(original));

  const Orientation values[] = {
    Orientation::None,
    Orientation::Left,
    Orientation::Right,
  };

  for (const auto value : values) {
    REQUIRE(board.setOrientation(value));

    Orientation readback = original;
    REQUIRE(board.getOrientation(readback));
    REQUIRE(readback == value);
  }

  REQUIRE(board.setOrientation(original));

  Orientation restored = Orientation::None;
  REQUIRE(board.getOrientation(restored));
  REQUIRE(restored == original);
}