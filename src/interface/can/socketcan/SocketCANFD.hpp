#pragma once

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sensorring_transport/MessageAssembler.hpp>
#include <sensorring_transport/MessageReassembler.hpp>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>
#include <string>
#include <vector>

#include "interface/can/CanInterface.hpp"

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @class SocketCANFD
 * @brief CAN communication class. This class uses a threaded listener and observer pattern notifying observer class
 * instances.
 * @author Stefan May, Hannes Duske
 * @date 13.05.2018 (modified 09.08.2024)
 */
class SocketCANFD : public CanInterface {
public:
  /**
   * Constructor
   * @param[in] interface_name device file link to CAN interface
   */
  SocketCANFD(std::string interface_name);

  /**
   * Constructor with full configuration.
   * @param[in] params SocketCAN configuration parameters.
   */
  explicit SocketCANFD(const SocketCanParams& params);

  /**
   * Destructor
   */
  ~SocketCANFD();

  /**
   * Open CAN interface.
   * @param[in] interface_name CAN interface name specified with slcand.
   * @return success==true
   */
  bool openInterface() override;

  /**
   * Send generic communication messsage.
   * @param[in] target ComEndpoint to which the message is sent.
   * @param[in] data Message payload.
   * @return success==true
   */
  bool send(ComEndpoint target, std::uint8_t command, const std::vector<uint8_t>& data) override;

  /**
   * Send CAN frame.
   * @param[in] canid ID of the can message.
   * @param[in] tx_buf Message payload.
   * @return success==true
   */
  bool sendCanFrame(std::uint32_t can_id, const std::vector<uint8_t>& data, bool fd);

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

  int _soc;
  sensorring::transport::MessageAssembler _assembler;
  sensorring::transport::MessageReassembler _reassembler;
};

} // namespace com

} // namespace sensorring

} // namespace eduart