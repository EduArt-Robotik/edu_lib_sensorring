#include "CanInterface.hpp"

namespace eduart {
namespace sensorring {
namespace com {

namespace {
bool filterMatches(const CanFilter& filter, std::uint32_t can_id) {
  if (filter.can_mask == 0U) {
    return true;
  }
  return (can_id & filter.can_mask) == (filter.can_id & filter.can_mask);
}
} // namespace

Subscription CanInterface::subscribeCanFrames(CanFrameCallback callback, CanFilter filter) {
  if (!callback) {
    return Subscription();
  }

  const auto token = SubscriberToken::getNextToken();
  {
    std::lock_guard<std::mutex> guard(_can_subscriber_mutex);
    _can_subscriptions.emplace(token, RawSubscriptionEntry{ std::move(callback), filter });
  }

  return Subscription(token, [this, token]() {
    std::lock_guard<std::mutex> guard(_can_subscriber_mutex);
    _can_subscriptions.erase(token);
  });
}

void CanInterface::dispatchCanFrame(const RawCanFrame& frame) {
  std::vector<CanFrameCallback> callbacks;
  {
    std::lock_guard<std::mutex> guard(_can_subscriber_mutex);
    callbacks.reserve(_can_subscriptions.size());
    for (const auto& [token, entry] : _can_subscriptions) {
      (void)token;
      if (!entry.callback) {
        continue;
      }
      if (filterMatches(entry.filter, frame.can_id)) {
        callbacks.push_back(entry.callback);
      }
    }
  }

  for (const auto& callback : callbacks) {
    try {
      callback(frame);
    } catch (...) {
    }
  }
}

} // namespace com
} // namespace sensorring
} // namespace eduart
