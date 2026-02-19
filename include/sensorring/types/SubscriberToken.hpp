// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SubscriberToken.hpp
 * @author EduArt Robotik GmbH
 * @brief  Opaque token identifying a subscription.
 * @date   2026-02-19
 */

#pragma once

#include <atomic>
#include <functional>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

/**
 * @brief Opaque token identifying a subscription (state or device group).
 */
struct SENSORRING_EXPORT SubscriberToken {
  using TokenType = unsigned long long;

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
    static std::atomic<TokenType> next_token{ 1 };
    return SubscriberToken(next_token++);
  }

  /**
   * @brief Underlying value for hashing and comparison.
   */
  TokenType value() const noexcept { return _value; }

  bool operator==(const SubscriberToken& other) const noexcept { return _value == other._value; }
  bool operator!=(const SubscriberToken& other) const noexcept { return _value != other._value; }

private:
  /**
   * @brief Construct from numeric value.
   */
  SubscriberToken(TokenType value) noexcept : _value(value) {};

  TokenType _value;
};

} // namespace eduart

#ifndef SWIG
namespace std {

template <> struct SENSORRING_EXPORT hash<eduart::SubscriberToken> {
  std::size_t operator()(const eduart::SubscriberToken& token) const noexcept { return std::hash<eduart::SubscriberToken::TokenType>{}(token.value()); }
};

} // namespace std
#endif // SWIG
