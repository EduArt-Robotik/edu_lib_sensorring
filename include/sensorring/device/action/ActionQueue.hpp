// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ActionQueue.hpp
 * @author EduArt Robotik GmbH
 * @brief  Queue to manage reusable actions.
 * @date   2025-07-28
 */

#pragma once

#include <deque>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "sensorring/device/action/Command.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensorring {

/**
 * @class ActionQueue
 * @brief Queue to manage reusable actions.
 */
class SENSORRING_EXPORT ActionQueue {
public:
  // Enqueue a standard non-replaceable action
  void enqueue(Action action) { _queue.push_back({ std::move(action), nullptr, {} }); }

  // Enqueue or update a replaceable action for a specific instance & key
  void enqueue(const ActionDispatcher* owner, ActionKey key, Action action) {
    auto lookupKey = std::make_pair(owner, key);

    auto it = indexMap_.find(lookupKey);
    if (it != indexMap_.end()) {
      // Action already exists in queue: replace its execution payload in-place
      it->second->execute = std::move(action);
    } else {
      // New action: add to queue and track its position in the map
      _queue.push_back({ std::move(action), owner, key });
      indexMap_[lookupKey] = --_queue.end();
    }
  }

  // Process all pending commands in FIFO order
  void processAll() {
    while (!_queue.empty()) {
      Command cmd = std::move(_queue.front());
      _queue.pop_front();

      // Clear map entry if this was a tracked replaceable action
      if (cmd.isReplaceable()) {
        indexMap_.erase({ cmd.owner, cmd.key });
      }

      try {
        cmd.execute();
      } catch (std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception occurred while executing action: " + std::string(e.what()));
      }
    }
  }

  static void clearGlobalQueue() { _global_queue = nullptr; }

  static void setGlobalQueue(ActionQueue* queue) { _global_queue = queue; }

  static ActionQueue* getGlobalQueue() { return _global_queue; }

private:
  // Pair hasher for unordered_map lookup
  struct PairHash {
    std::size_t operator()(const std::pair<const ActionDispatcher*, ActionKey>& p) const {
      auto h1 = std::hash<const ActionDispatcher*>{}(p.first);
      auto h2 = std::hash<ActionKey>{}(p.second);
      return h1 ^ (h2 << 1);
    }
  };

  std::deque<Command> _queue;
  inline static ActionQueue* _global_queue = nullptr;
  std::unordered_map<std::pair<const ActionDispatcher*, ActionKey>, std::deque<Command>::iterator, PairHash> indexMap_;
};

} // namespace sensorring

} // namespace eduart