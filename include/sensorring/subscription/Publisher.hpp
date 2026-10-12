// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Publisher.hpp
 * @author EduArt Robotik GmbH
 * @brief  Thread-safe, deadlock-free publish/subscribe template.
 * @date   2026-03-27
 */

#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "sensorring/subscription/SubscriberToken.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace subscription {

/**
 * @class Publisher
 * @brief Generic thread-safe publisher that manages callback subscriptions.
 *
 * Stores subscriber callbacks keyed by SubscriberToken.  The publish() method
 * copies the callback list under lock and invokes the copies outside the lock
 * (copy-then-invoke), preventing deadlocks when a callback subscribes or
 * unsubscribes.
 *
 * @tparam Args Parameter types forwarded to subscriber callbacks.
 */
template <typename... Args> class Publisher {
public:
  /// Callback type for subscribers.
  using Callback = std::function<void(Args...)>;

  Publisher() = default;

  /**
   * @brief Register a callback.
   * @param[in] callback Invoked on each publish().
   * @return RAII Subscription that auto-cancels on destruction.
   */
  Subscription subscribe(Callback callback) {
    if (!callback) {
      return Subscription();
    }
    auto token = SubscriberToken::getNextToken();
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _subscribers.emplace(token, std::move(callback));
    }
    return Subscription(token, [this, token]() {
      unsubscribe(token);
    });
  }

  /**
   * @brief Remove a previously registered callback.
   * @param[in] token Token identifying the subscription to cancel.
   */
  void unsubscribe(SubscriberToken token) {
    std::lock_guard<std::mutex> lock(_mutex);
    _subscribers.erase(token);
  }

  /**
   * @brief Invoke all registered callbacks with the given arguments.
   *
   * Callbacks are copied under lock and invoked outside the lock.
   * Exceptions thrown by callbacks are silently absorbed.
   *
   * @param[in] args Arguments forwarded to each callback.
   */
  void publish(Args... args) const {
    auto callbacks = copySubscribers();
    for (const auto& cb : callbacks) {
      try {
        cb(args...);
      } catch (...) {
      }
    }
  }

  /**
   * @brief Copy the current subscriber list under lock.
   *
   * Useful when the caller needs custom exception handling or filtering
   * around the invocation loop.
   *
   * @return Vector of callback copies.
   */
  std::vector<Callback> copySubscribers() const {
    std::vector<Callback> callbacks;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      callbacks.reserve(_subscribers.size());
      for (const auto& [token, cb] : _subscribers) {
        if (cb) {
          callbacks.push_back(cb);
        }
      }
    }
    return callbacks;
  }

private:
  mutable std::mutex _mutex;
  std::unordered_map<SubscriberToken, Callback> _subscribers;
};

} // namespace subscription

} // namespace sensorring

} // namespace eduart
