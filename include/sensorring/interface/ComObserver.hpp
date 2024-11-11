#pragma once

#include <vector>
#include <memory>
#include "Endpoints.hpp"

namespace com
{

/**
 * @class ComObserver
 * @brief Abstract communication observer class. Derived classes get notified when new messages arrive.
 * @author Hannes Duske
 * @date 11.11.2024
 */
class ComObserver
{
public:
  /**
   * Constructor
   */
  ComObserver();

  /**
   * Destructor
   */
  virtual ~ComObserver();

  /**
   * Set CAN bus identifier
   * @param[in] id CAN ID
   */
  void addEndpoint(const Endpoint target);

  /**
   * Get CAN bus identifier
   * @return CAN ID
   */
  const std::vector<Endpoint>& getEndpoints() const;

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
  void forwardNotification(const Endpoint source, const std::vector<uint8_t>& data);
  
  /**
   * Interface declaration for implementation through inherited classes.
   * @params[in] frame CAN frame
   */
  virtual void notify(const Endpoint source, const std::vector<uint8_t>& data) = 0;

private:

  std::vector<Endpoint> _canids;

  long _seconds;
  
  long _usec;
};

} // namespace
