// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ActionDispatcher.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base class that executes actions either immediately or via a managed queue.
 * @date   2025-07-28
 */

#pragma once

#include <deque>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "sensorring/device/action/ActionQueue.hpp"
#include "sensorring/device/action/Command.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensorring {

/**
 * @class ActionDispatcher
 * @brief Base class that executes actions either immediately or via a managed queue.
 */
class SENSORRING_EXPORT ActionDispatcher {
public:
  virtual ~ActionDispatcher() = default;

  void setQueue(ActionQueue* queue) { _queue = queue; }

  void clearQueue() { _queue = nullptr; }

  bool isManaged() const { return _queue != nullptr; }

  void execute(Action action) {
    if (_queue) {
      _queue->enqueue(this, "", std::move(action));
    } else {
      try {
        action();
      } catch (std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception occurred while executing action: " + std::string(e.what()));
      }
    }
  }

  void execute(std::string_view key, Action action) {
    if (_queue) {
      _queue->enqueue(this, key, std::move(action));
    } else {
      try {
        action();
      } catch (std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception occurred while executing action: " + std::string(e.what()));
      }
    }
  }

  static void globalExecute(Action action) {
    if (ActionQueue::getGlobalQueue()) {
      ActionQueue::getGlobalQueue()->enqueue(std::move(action));
    } else {
      try {
        action();
      } catch (std::exception& e) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Exception occurred while executing global action: " + std::string(e.what()));
      }
    }
  }

private:
  ActionQueue* _queue = nullptr;
};

} // namespace sensorring

} // namespace eduart