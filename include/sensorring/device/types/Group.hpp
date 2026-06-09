// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Group.hpp
 * @author EduArt Robotik GmbH
 * @brief  Typed container for a group of devices with subscribe() and iteration.
 * @date   2026-05-08
 */

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "sensorring/subscription/SubscriberToken.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class Group
 * @brief Typed wrapper over a vector of device pointers.
 *
 * Provides iteration (range-for yields T&) and a subscribe() method that
 * subscribes to all devices in the group at once. Returned Subscription
 * manages all individual subscriptions and cancels them collectively.
 *
 * @tparam T Device interface type (DepthSensor, ThermalSensor, Light, …).
 */
template <typename T> class Group {
public:
  /** Dereferencing iterator so range-for yields T& instead of T*. */
  class iterator {
    typename std::vector<T*>::iterator _it;

  public:
    iterator(typename std::vector<T*>::iterator it)
        : _it(it) {}
    T& operator*() const { return **_it; }
    T* operator->() const { return *_it; }
    iterator& operator++() {
      ++_it;
      return *this;
    }
    bool operator!=(const iterator& other) const { return _it != other._it; }
    bool operator==(const iterator& other) const { return _it == other._it; }
  };

  /**
   * @class const_iterator
   * @brief Const dereferencing iterator so range-for over a const Group<T> yields const T&.
   */
  class const_iterator {
    typename std::vector<T*>::const_iterator _it;

  public:
    const_iterator(typename std::vector<T*>::const_iterator it)
        : _it(it) {}
    const T& operator*() const { return **_it; }
    const T* operator->() const { return *_it; }
    const_iterator& operator++() {
      ++_it;
      return *this;
    }
    bool operator!=(const const_iterator& other) const { return _it != other._it; }
    bool operator==(const const_iterator& other) const { return _it == other._it; }
  };

  Group() = default;
  explicit Group(std::vector<T*> devices)
      : _devices(std::move(devices)) {}

  /// Access device by index (returns reference).
  T& operator[](std::size_t i) { return *_devices[i]; }
  const T& operator[](std::size_t i) const { return *_devices[i]; }

  /// Number of devices in this group.
  std::size_t size() const { return _devices.size(); }

  /// Whether the group is empty.
  bool empty() const { return _devices.empty(); }

  /// Begin iterator (mutable).
  iterator begin() { return iterator(_devices.begin()); }
  /// Past-the-end iterator (mutable).
  iterator end() { return iterator(_devices.end()); }
  /// Begin iterator (const).
  const_iterator begin() const { return const_iterator(_devices.begin()); }
  /// Past-the-end iterator (const).
  const_iterator end() const { return const_iterator(_devices.end()); }

  /**
   * @brief Subscribe to all devices in the group.
   *
   * Returns a single Subscription that manages all individual subscriptions.
   * When cancelled or destroyed, all subscriptions are cancelled.
   *
   * Only available when T defines a MeasurementType alias (e.g. DepthSensor, ThermalSensor).
   *
   * @param[in] callback Invoked for each measurement from each device.
   * @return RAII Subscription managing all individual subscriptions.
   */
  template <typename U = T, typename = typename U::MeasurementType> subscription::Subscription subscribe(std::function<void(const typename U::MeasurementType&)> callback) {
    auto subs = std::make_shared<std::vector<subscription::Subscription> >();
    for (auto* dev : _devices) {
      subs->push_back(dev->subscribe(callback));
    }
    auto token = subscription::SubscriberToken::getNextToken();
    return subscription::Subscription(token, [subs]() {
      for (auto& s : *subs)
        s.cancel();
      subs->clear();
    });
  }

  /**
   * @brief Subscribe to all devices in the group with synchronized frame delivery.
   *
   * The callback is invoked once per complete frame — i.e. only after every device in the
   * group has produced a new measurement. The callback receives a vector containing exactly
   * one measurement per device (ordered by device index). The effective callback rate equals
   * the rate of the slowest device in the group.
   *
   * Only available when T defines a MeasurementType alias (e.g. DepthSensor, ThermalSensor).
   *
   * @param[in] callback Invoked with a complete frame (one measurement per device, ordered by index).
   * @return RAII Subscription managing all individual subscriptions.
   */
  template <typename U = T, typename = typename U::MeasurementType> subscription::Subscription subscribeAll(std::function<void(const std::vector<typename U::MeasurementType>&)> callback) {
    const std::size_t n = _devices.size();

    struct FrameState {
      std::vector<typename U::MeasurementType> buffer;
      std::vector<bool> received;
      std::size_t count    = 0;
      std::size_t expected = 0;
      std::function<void(const std::vector<typename U::MeasurementType>&)> callback;
    };

    auto state = std::make_shared<FrameState>();
    state->buffer.resize(n);
    state->received.resize(n, false);
    state->count    = 0;
    state->expected = n;
    state->callback = std::move(callback);

    auto subs = std::make_shared<std::vector<subscription::Subscription> >();
    for (std::size_t i = 0; i < n; ++i) {
      subs->push_back(_devices[i]->subscribe([state, i](const typename U::MeasurementType& m) {
        state->buffer[i] = m;
        if (!state->received[i]) {
          state->received[i] = true;
          state->count++;
        }
        if (state->count == state->expected) {
          state->callback(state->buffer);
          // Reset for next frame
          std::fill(state->received.begin(), state->received.end(), false);
          state->count = 0;
        }
      }));
    }

    auto token = subscription::SubscriberToken::getNextToken();
    return subscription::Subscription(token, [subs]() {
      for (auto& s : *subs)
        s.cancel();
      subs->clear();
    });
  }

private:
  std::vector<T*> _devices;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
