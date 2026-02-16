#pragma once

#include <chrono>
#include <unordered_set>
#include <vector>

#include "sensorring/interface/ComEndpoint.hpp"

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
   * @brief Constructor
   */
  ComObserver();

  /**
   * @brief Destructor
   */
  virtual ~ComObserver();

  /**
   * @brief Add a ComEndpoint to the list of observed endpoints.
   * @param[in] target ComEndpoint which will trigger the notify callback on future messages.
   * @return returns true if the new endpoint was added successfully
   */
  bool subscribeToEndpoint(const ComEndpoint target);

  /**
   * @brief Remove a ComEndpoint from the list of observed endpoints.
   * @param[in] target ComEndpoint which will no longer trigger the notify callback on future messages.
   * @return returns true if the endpoint was removed successfully
   */
  bool removeEndpoint(const ComEndpoint target);

  /**
   * @brief Get a list of all ComEndpoints that currently trigger the notify callback.
   * @return Vector of all subscribed endpoints.
   */
  const std::unordered_set<ComEndpoint>& getEndpoints() const;

  /**
   * @brief Check connection status, i.e., whether the elapsed time since the last message arrival is smaller than a specific timeout.
   * @param[in] timeoutInMillis timeout in milliseconds
   * @return connection status
   */
  bool checkConnectionStatus(unsigned int timeoutInMillis = 100);

  /**
   * @brief Distribute new can frame to all registered observers
   * @param[in] source ComEndpoint that sent the message
   * @param[in] data Message payload
   */
  void forwardNotification(const ComEndpoint source, const std::vector<uint8_t>& data);

  /**
   * @brief Interface declaration for implementation through inherited classes.
   * @param[in] source ComEndpoint that sent the message
   * @param[in] data Message payload
   */
  virtual void comCallback(const ComEndpoint source, const std::vector<uint8_t>& data) = 0;

private:
  std::unordered_set<ComEndpoint> _endpoints;

  std::chrono::time_point<std::chrono::steady_clock> _timestamp;
};

} // namespace com

} // namespace eduart