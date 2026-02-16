#pragma once

#include <atomic>
#include <functional>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

/**
 * @brief Opaque token identifying a subscription (state or device group).
 */
struct SENSORRING_EXPORT SubscriberToken {

  /**
   * @brief Default construction (invalid token, value 0).
   */
  SubscriberToken() noexcept : _value(0) {}

  /**
   * @brief Check if the token is valid (non-zero).
   * @return true if the token represents a valid subscription.
   */
  bool isValid() const noexcept { return _value != 0; }

  /**
   * @brief Get the next token.
   */
  static SubscriberToken getNextToken() noexcept {
    static std::atomic<unsigned int> next_token{ 1 };
    return SubscriberToken(next_token++);
  }

  /**
   * @brief Underlying value for hashing and comparison.
   */
  unsigned int value() const noexcept { return _value; }

  bool operator==(const SubscriberToken& other) const noexcept { return _value == other._value; }
  bool operator!=(const SubscriberToken& other) const noexcept { return _value != other._value; }

private:
  /**
   * @brief Construct from numeric value.
   */
  SubscriberToken(unsigned int value) noexcept : _value(value) {};

  unsigned int _value;

};

} // namespace eduart

namespace std {

template <> struct hash<eduart::SubscriberToken> {
  std::size_t operator()(const eduart::SubscriberToken& token) const noexcept { return std::hash<unsigned int>{}(token.value()); }
};

} // namespace std
