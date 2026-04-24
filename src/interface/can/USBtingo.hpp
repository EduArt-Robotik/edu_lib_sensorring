#pragma once

#include <memory>
#include <sensorring_transport/MessageAssembler.hpp>
#include <sensorring_transport/MessageReassembler.hpp>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>
#include <string>
#include <usbtingo/device/Device.hpp>
#include <vector>

#include "CanInterface.hpp"

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @class USBtingo
 * @brief CAN communication class using the USBtingo USB to CANFD converter.
 * @author Hannes Duske
 * @date 29.01.2025
 */
class USBtingo : public CanInterface {
public:
  /**
   * Constructor
   * @param[in] serial serial number of the USBtingo to use
   */
  USBtingo(std::string serial);

  /**
   * Destructor
   */
  ~USBtingo();

  /**
   * Send generic communication messsage.
   * @param[in] target ComEndpoint to which the message is sent.
   * @param[in] data Message payload.
   * @return success==true
   */
  bool send(ComEndpoint target, std::uint8_t command, const std::vector<uint8_t>& data) override;

  /**
   * Send raw CAN/CAN-FD frame.
   * @note USBtingo transmits CAN-FD only; the fd flag is accepted and ignored.
   */
  bool sendCanFrame(std::uint32_t can_id, const std::vector<uint8_t>& data, bool fd) override;

  /**
   * Open CAN interface.
   * @return success==true
   */
  bool openInterface() override;

  /**
   * Close device file link.
   * @return success==true
   */
  bool closeInterface() override;

  /**
   * Repair the connection in case of an error.
   * @return success==true
   */
  bool repairInterface() override;

private:
  bool listener() override;

  std::unique_ptr<usbtingo::device::Device> _dev;
  sensorring::transport::MessageAssembler _assembler;
  sensorring::transport::MessageReassembler _reassembler;
};

} // namespace com

} // namespace sensorring

} // namespace eduart