#include "sensorring_transport/can/CanCodec.hpp"

using namespace eduart::transport::protocol;
namespace eduart {
namespace sensorring {
namespace transport {
namespace can {

// ── CAN ID helpers ──────────────────────────────────────────────────

std::uint32_t CanCodec::makeCanId(std::uint8_t direction, std::uint8_t boardAddress) {
  return (SYSID_SENSOR_RING << 8) | (static_cast<std::uint32_t>(direction) << 7) | boardAddress;
}

void CanCodec::makeCanIds(std::uint8_t boardAddress, std::uint32_t& inputId, std::uint32_t& outputId) {
  inputId  = (SYSID_SENSOR_RING << 8) | (0u << 7) | boardAddress;
  outputId = (SYSID_SENSOR_RING << 8) | (1u << 7) | boardAddress;
}

std::uint8_t CanCodec::extractBoardAddress(std::uint32_t canId) {
  return static_cast<std::uint8_t>(canId & 0x7F);
}

std::uint8_t CanCodec::extractDirection(std::uint32_t canId) {
  return static_cast<std::uint8_t>((canId >> 7) & 1);
}

// ── Fragment byte helpers ───────────────────────────────────────────

std::uint8_t CanCodec::makeFragmentByte(std::uint8_t type, std::uint8_t count) {
  return static_cast<std::uint8_t>((type & fragment::TYPE_MASK) | (count & fragment::COUNT_MASK));
}

std::uint8_t CanCodec::fragmentType(std::uint8_t fragmentByte) {
  return fragmentByte & fragment::TYPE_MASK;
}

std::uint8_t CanCodec::fragmentCount(std::uint8_t fragmentByte) {
  return fragmentByte & fragment::COUNT_MASK;
}

// ── Encode / Decode ─────────────────────────────────────────────────

CanFrame CanCodec::encode(const TransportFrame& frame) {
  CanFrame canFrame;
  canFrame.id = makeCanId(static_cast<std::uint8_t>(frame.direction), frame.boardAddress);

  canFrame.data.reserve(HEADER_SIZE + frame.data.size());
  canFrame.data.push_back(makeFragmentByte(frame.fragmentType, frame.fragmentCount));
  canFrame.data.push_back(frame.deviceId);
  canFrame.data.push_back(frame.command);
  canFrame.data.insert(canFrame.data.end(), frame.data.begin(), frame.data.end());

  return canFrame;
}

TransportFrame CanCodec::decode(std::uint32_t canId, const std::uint8_t* data, std::size_t dataLen) {
  TransportFrame frame;
  frame.direction    = static_cast<Direction>(extractDirection(canId));
  frame.boardAddress = extractBoardAddress(canId);

  if (dataLen >= HEADER_SIZE) {
    std::uint8_t fragByte = data[0];
    frame.deviceId        = data[1];
    frame.command         = data[2];
    frame.fragmentType    = fragmentType(fragByte);
    frame.fragmentCount   = fragmentCount(fragByte);

    std::size_t payloadLen = dataLen - HEADER_SIZE;
    frame.data.assign(data + HEADER_SIZE, data + HEADER_SIZE + payloadLen);
  }

  return frame;
}

} // namespace can
} // namespace transport
} // namespace sensorring
} // namespace eduart