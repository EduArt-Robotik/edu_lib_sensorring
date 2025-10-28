#pragma once

#include <linux/can.h>
#include <linux/can/raw.h>
#include <map>
#include <string>
#include <vector>

#include "interface/ComInterface.hpp"

namespace eduart {

namespace com {

/**
 * @class SocketCANFD
 * @brief CAN communication class. This class uses a threaded listener and observer pattern notifying observer class
 * instances.
 * @author Stefan May, Hannes Duske
 * @date 13.05.2018 (modified 09.08.2024)
 */
class SocketCANFD : public ComInterface {
public:
  /**
   * Constructor
   * @param[in] interface_name device file link to CAN interface
   */
  SocketCANFD(std::string interface_name);

  /**
   * Destructor
   */
  ~SocketCANFD();

  /**
   * Open CAN interface.
   * @param[in] interface_name CAN interface name specified with slcand.
   * @return success==true
   */
  bool openInterface(std::string interface_name) override;

  /**
   * Send generic communication messsage.
   * @param[in] target ComEndpoint to which the message is sent.
   * @param[in] data Message payload.
   * @return success==true
   */
  bool send(ComEndpoint target, const std::vector<uint8_t>& data) override;

  /**
   * Send CAN frame.
   * @param[in] canid ID of the can message.
   * @param[in] tx_buf Message payload.
   * @return success==true
   */
  bool send(canid_t canid, const std::vector<uint8_t>& tx_buf);

  /**
   * Send CAN frame.
   * @param[in] frame CAN frame.
   * @return success==true
   */
  bool send(const canfd_frame* frame);

  /**
   * Close device file link.
   * @return success==true
   */
  bool closeInterface() override;

  /**
   * Add endpoint for a new tof sensor
   * @param[in] idx index of the sensor
   */
  void addToFSensorToEndpointMap(std::size_t idx) override;

  /**
   * Add endpoint for a new thermal sensor
   * @param[in] idx index of the sensor
   */
  void addThermalSensorToEndpointMap(std::size_t idx) override;

private:
  void fillEndpointMap();

  canid_t mapEndpointToId(ComEndpoint endpoint);

  ComEndpoint mapIdToEndpoint(canid_t id);

  std::map<ComEndpoint, canid_t> _id_map;

  bool listener() override;

  int _soc;
};

} // namespace com

} // namespace eduart