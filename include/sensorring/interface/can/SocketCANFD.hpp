#pragma once

#include <linux/can.h>
#include <linux/can/raw.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>

#include "SocketCANFDObserver.hpp"

namespace com
{

/**
 * @class SocketCANFD
 * @brief CAN communication class. This class uses a threaded listener and observer pattern notifying observer class instances.
 * @author Stefan May, Hannes Duske
 * @date 13.05.2018 (modified 09.08.2024)
 */
class SocketCANFD
{
public:
  /**
   * Constructor
   * @param[in] devFile device file link to CAN interface
   */
  SocketCANFD(std::string devFile);

  /**
   * Destructor
   */
  ~SocketCANFD();

  /**
   * The SocketCANFD class instance reads all CAN data packets and distributes them according to the observer IDs.
   * @param[in] observer Observer instance, which should be notified when data is available.
   * @return success==true
   */
  std::string getInterfaceName();

  /**
   * The SocketCANFD class instance reads all CAN data packets and distributes them according to the observer IDs.
   * @param[in] observer Observer instance, which should be notified when data is available.
   * @return success==true
   */
  bool registerObserver(SocketCANFDObserver* observer);

  /**
   * Remove all registered observers
   */
  void clearObservers();

  /**
   * Open CAN interface.
   * @param[in] port CAN interface name specified with slcand.
   * @return success==true
   */
  bool openPort(const char* port);

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
   * Start listener thread.
   * @return success==true, failure==false (e.g. when listener is already running)
   */
  bool startListener();

  /**
   * Terminate listener thread
   */
  void stopListener();

  /**
   * Close device file link.
   * @return success==true
   */
  bool closePort();

private:

  bool listener();

  int _soc;

  bool _listenerIsRunning;

  bool _shutDownListener;

  std::vector<SocketCANFDObserver*> _observers;

  std::unique_ptr<std::thread> _thread;

  std::mutex _mutex;

  std::string _devFile;
};

} // namespace
