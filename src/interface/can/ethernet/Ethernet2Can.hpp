#pragma once

#include <cstdint>
#include <sensorring_transport/MessageAssembler.hpp>
#include <sensorring_transport/MessageReassembler.hpp>
#include <string>
#include <vector>

#include "interface/can/CanInterface.hpp"

namespace eduart {
namespace sensorring {
namespace com {

/**
 * @brief CAN interface routed through a UDP Ethernet gateway.
 */
class Ethernet2Can : public CanInterface {
public:
  explicit Ethernet2Can(const Ethernet2CanParams& params);
  ~Ethernet2Can() override;

  bool send(ComEndpoint target, std::uint8_t command, const std::vector<std::uint8_t>& data) override;
  bool openInterface() override;
  bool closeInterface() override;
  bool repairInterface() override;

private:
  bool listener() override;
  bool sendCanFrame(std::uint32_t can_id, const std::vector<std::uint8_t>& data);

  Ethernet2CanParams _ethernet_params;
  std::intptr_t _socket;
  sensorring::transport::MessageAssembler _assembler;
  sensorring::transport::MessageReassembler _reassembler;
};

} // namespace com
} // namespace sensorring
} // namespace eduart
