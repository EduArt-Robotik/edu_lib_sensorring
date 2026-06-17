#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/subscription/SubscriberToken.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {

using subscription::SubscriberToken;
using subscription::Subscription;

class ComInterface {

public:
  /// Callback type for communication subscriptions.
  using ComCallback = std::function<void(const ComEndpoint&, std::uint8_t command, const std::vector<std::uint8_t>&)>;

  /**
   * Constructor
   */
  ComInterface(ComInterfaceID id);

  /**
   * Destructor
   */
  virtual ~ComInterface();

  /**
   * Get the ComInterfaceID of the ComInterface object
   * @return ComInterfaceID of the interface
   */
  ComInterfaceID getID() const;

  /**
   * @brief Subscribe to incoming messages on this interface.
   *
   * The callback is invoked from the listener thread for every incoming message.
   * The returned Subscription automatically unsubscribes when it goes out of scope.
   *
   * @param[in] callback  Invoked with (source endpoint, payload) for each message.
   * @param[in] endpoints If non-empty, only messages from these endpoints trigger the callback.
   *                      An empty set means all messages are forwarded.
   * @return RAII Subscription that auto-cancels on destruction.
   */
  Subscription subscribe(ComCallback callback, std::vector<ComEndpoint> endpoints = {});

  /**
   * @brief Unsubscribe a previously registered callback.
   * @param[in] token Token identifying the subscription to cancel.
   */
  void unsubscribe(SubscriberToken token);

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
   * Send a generic communication message.
   * @param[in] target ComEndpoint to which the message is sent.
   * @param[in] command Command byte.
   * @param[in] data Payload bytes (without command).
   */
  virtual bool send(ComEndpoint target, std::uint8_t command, const std::vector<std::uint8_t>& data) = 0;

  /**
   * Open the communication interface.
   * @return success==true
   */
  virtual bool openInterface() = 0;

  /**
   * Close the communication interface.
   * @return success==true
   */
  virtual bool closeInterface() = 0;

  /**
   * Repair the connection in case of an error.
   * @return success==true
   */
  virtual bool repairInterface() = 0;

  /**
   * Check if a communication error has occurred.
   * @return error==true
   */
  bool hasError() const;

  /**
   * @brief Perform any necessary configuration after opening the interface or boards.
   */
  virtual bool configure();

protected:
  using Mutex     = std::mutex;
  using LockGuard = std::lock_guard<Mutex>;

  virtual bool listener() = 0;

  /**
   * @brief Dispatch an incoming message to all registered subscribers.
   *
   * @param[in] source  Endpoint that the message was received from.
   * @param[in] command Command byte.
   * @param[in] data    Payload bytes.
   */
  void dispatchMessage(const ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data);

  std::atomic<bool> _communication_error;

  std::atomic<bool> _listener_is_running;

  std::atomic<bool> _shut_down_listener;

  Mutex _mutex;

  ComInterfaceID _id;

private:
  struct SubscriptionEntry {
    ComCallback callback;
    std::unordered_set<ComEndpoint> endpoints;
  };

  std::mutex _subscriber_mutex;
  std::unordered_map<SubscriberToken, SubscriptionEntry> _com_subscriptions;

  std::unique_ptr<std::thread> _thread;
};

} // namespace com

} // namespace sensorring

} // namespace eduart