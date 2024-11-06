#pragma once

#include <linux/can.h>
#include <linux/can/raw.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <map>

#include "ComInterface.hpp"
#include "ComObserver.hpp"

namespace com
{

/**
 * @class SocketCANFD
 * @brief CAN communication class. This class uses a threaded listener and observer pattern notifying observer class instances.
 * @author Stefan May, Hannes Duske
 * @date 13.05.2018 (modified 09.08.2024)
 */
class SocketCANFD : public ComInterface
{
public:
  /**
   * Constructor
   * @param[in] devFile device file link to CAN interface
   */
  SocketCANFD(std::string interface_name, std::size_t sensor_count);

  /**
   * Destructor
   */
  ~SocketCANFD();

  /**
   * Open CAN interface.
   * @param[in] port CAN interface name specified with slcand.
   * @return success==true
   */
  bool openInterface(std::string interface_name) override;

  /**
   * Send CAN frame.
   * @param[in] frame CAN frame
   */
  bool send(ComEndpoint target, const std::vector<uint8_t>& data) override;

  /**
   * Send CAN frame.
   * @param[in] frame CAN frame
   */
  bool send(canid_t canid, const std::vector<uint8_t>& tx_buf);

  /**
   * Send CAN frame.
   * @param[in] frame CAN frame
   */
  bool send(const canfd_frame* frame);

  /**
   * Close device file link.
   * @return success==true
   */
  bool closeInterface() override;

private:

  void fillMap(std::size_t sensor_count);

  canid_t mapEndpointToId(ComEndpoint endpoint);
  
  //ComEndpoint mapIdToEndpoint(canid_t id);

  std::map<ComEndpoint, canid_t> _id_map;

  bool listener() override;

  int _soc;
};

} // namespace
