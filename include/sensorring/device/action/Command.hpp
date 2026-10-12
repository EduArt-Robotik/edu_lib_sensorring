// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Command.hpp
 * @author EduArt Robotik GmbH
 * @brief  Command structure for reusable actions.
 * @date   2025-07-28
 */

#pragma once

#include <deque>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace eduart {

namespace sensorring {

using Action    = std::function<void()>;
using ActionKey = std::string_view;

class ActionDispatcher;

struct Command {
  /// Callable to execute the action.
  Action execute;

  /// Owner of the command used to identify replaceable actions. If owner == nullptr, this task is non-replaceable.
  const ActionDispatcher* owner = nullptr;

  /// Key used to identify replaceable actions for a specific owner instance. If key is empty, this task is non-replaceable.
  ActionKey key{};

  /// Returns true if this command is replaceable (i.e. has a non-null owner and a non-empty key).
  bool isReplaceable() const { return owner != nullptr && !key.empty(); }
};

} // namespace sensorring

} // namespace eduart