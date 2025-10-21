#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "ComEndpoints.hpp"

namespace eduart {

namespace com {

/**
 * @class ComObserver
 * @brief Abstract communication observer class. Derived classes get notified when new messages arrive.
 * @author Hannes Duske
 * @date 11.11.2024
 */
class ComObserver {
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
   * Add a ComEndpoint to the list of observed endpoints.
   * @param[in] target ComEndpoint which will trigger the notify callback on future messages.
   */
  void addEndpoint(const ComEndpoint target);

  /**
   * Get a list of all ComEndpoints that currently trigger the notify callback.
   * @return Vector of all subscribed endpoints.
   */
  const std::vector<ComEndpoint>& getEndpoints() const;

  /**
   * Check connection status, i.e., whether the elapsed time since the last message arrival is smaler than a specific timeout.
   * @param timeoutInMillis timeout in milliseconds
   * @return connection status
   */
  bool checkConnectionStatus(unsigned int timeoutInMillis = 100);

  /**
   * Distribute new can frame to all registered observers
   * @param[in] source ComEndpoint that sent the message
   * @param[in] data Message payload
   */
  void forwardNotification(const ComEndpoint source, const std::vector<uint8_t>& data);

  /**
   * Interface declaration for implementation through inherited classes.
   * @param[in] source ComEndpoint that sent the message
   * @param[in] data Message payload
   */
  virtual void notify(const ComEndpoint source, const std::vector<uint8_t>& data) = 0;

private:
  std::vector<ComEndpoint> _endpoints;

  std::chrono::time_point<std::chrono::steady_clock> _timestamp;
};

}

}