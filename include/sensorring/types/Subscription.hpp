// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Subscription.hpp
 * @author EduArt Robotik GmbH
 * @brief  RAII wrapper pairing a SubscriberToken with its cancel function.
 * @date   2026-03-25
 */

#pragma once

#include <functional>
#include <vector>

#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/SubscriberToken.hpp"

namespace eduart {

/**
 * @class Subscription
 * @brief Move-only RAII wrapper that pairs a SubscriberToken with an unsubscribe
 *        callable. Destroying a Subscription automatically cancels it.
 *
 * Observables (MeasurementManager, Logger, …) return a Subscription from their
 * subscribe methods. The holder can call cancel() explicitly or simply let the
 * Subscription go out of scope.
 */
class SENSORRING_EXPORT Subscription {
public:
  /**
   * @brief Default-construct an inactive Subscription.
   */
  Subscription() noexcept = default;

  /**
   * @brief Construct from a token and an unsubscribe callable.
   * @param[in] token        The token identifying the subscription.
   * @param[in] unsubscribe  Callable that removes the subscription from the observable.
   */
  Subscription(SubscriberToken token, std::function<void()> unsubscribe)
      : _token(token)
      , _unsubscribe(std::move(unsubscribe)) {}

  // Move-only — prevents double-cancel from copies.
  Subscription(Subscription&& other) noexcept : _token(other._token), _unsubscribe(std::move(other._unsubscribe)) {
    other._token       = SubscriberToken{};
    other._unsubscribe = nullptr;
  }

  Subscription& operator=(Subscription&& other) noexcept {
    if (this != &other) {
      cancel();
      _token             = other._token;
      _unsubscribe       = std::move(other._unsubscribe);
      other._token       = SubscriberToken{};
      other._unsubscribe = nullptr;
    }
    return *this;
  }

  Subscription(const Subscription&)            = delete;
  Subscription& operator=(const Subscription&) = delete;

  /**
   * @brief Destructor — automatically cancels an active subscription.
   */
  ~Subscription() { cancel(); }

  /**
   * @brief Cancel the subscription. Safe to call multiple times (idempotent).
   */
  void cancel() noexcept {
    if (_unsubscribe) {
      _unsubscribe();
      _unsubscribe = nullptr;
    }
    _token = SubscriberToken{};
  }

  /**
   * @brief Check whether this subscription is still active.
   * @return true if the subscription has not been cancelled.
   */
  bool isActive() const noexcept { return _token.isValid(); }

  /**
   * @brief Access the underlying token (e.g. for logging or debugging).
   * @return The SubscriberToken associated with this subscription.
   */
  SubscriberToken token() const noexcept { return _token; }

private:
  SubscriberToken _token;
  std::function<void()> _unsubscribe;
};

} // namespace eduart
