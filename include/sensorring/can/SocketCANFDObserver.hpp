#ifndef _SOCKETCANFDOBSERVER_H_
#define _SOCKETCANFDOBSERVER_H_

#include <linux/can.h>
#include <vector>
#include <memory>

namespace edu
{

/**
 * @class SocketCANFDObserver
 * @brief Abstract observer class. Derived classes get notified according their CAN bus identifiers.
 * @author Stefan May, Hannes Duske
 * @date 13.05.2018 (modified 09.08.2024)
 */
class SocketCANFDObserver
{
public:
  /**
   * Constructor
   */
  SocketCANFDObserver();

  /**
   * Destructor
   */
  virtual ~SocketCANFDObserver();

  /**
   * Set CAN bus identifier
   * @param[in] id CAN ID
   */
  void addCANId(const canid_t id);

  /**
   * Get CAN bus identifier
   * @return CAN ID
   */
  const std::vector<canid_t>& getCANIds() const;

  /**
   * Check connection status, i.e., whether the elapsed time since the last message arrival is smaler than a specific timeout.
   * @param timeoutInMillis timeout in milliseconds
   * @return connection status
   */
  bool checkConnectionStatus(unsigned int timeoutInMillis=100);
  
   /**
   * Distribute new can frame to all registered observers
   * @param[in] frame CANFD Frame that will be distributed
   */
  void forwardNotification(const canfd_frame& frame);
  
  /**
   * Interface declaration for implementation through inherited classes.
   * @params[in] frame CAN frame
   */
  virtual void notify(const canfd_frame& frame) = 0;

private:

  std::vector<canid_t> _canids;

  long _seconds;
  
  long _usec;
};

} // namespace

#endif // _SocketCANFDOBSERVER_H_
