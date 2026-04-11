#include "ComInterface.hpp"

namespace eduart {

namespace sensorring {

namespace com {

ComInterface::ComInterface(ComInterfaceID id)
    : _communication_error(false)
    , _listener_is_running(false)
    , _shut_down_listener(false)
    , _id(id)
    , _thread{ nullptr } {
}

ComInterface::~ComInterface() {
  stopListener();
}

ComInterfaceID ComInterface::getID() const {
  return _id;
}

Subscription ComInterface::subscribe(ComCallback callback, std::vector<ComEndpoint> endpoints) {
  if (!callback) {
    return Subscription();
  }
  auto token = SubscriberToken::getNextToken();
  {
    std::lock_guard<std::mutex> guard(_subscriber_mutex);
    _com_subscriptions.emplace(
        token, SubscriptionEntry{
                   std::move(callback), { endpoints.begin(), endpoints.end() }
    });
  }
  return Subscription(token, [this, token]() {
    unsubscribe(token);
  });
}

void ComInterface::unsubscribe(SubscriberToken token) {
  std::lock_guard<std::mutex> guard(_subscriber_mutex);
  _com_subscriptions.erase(token);
}

void ComInterface::dispatchMessage(const ComEndpoint& source, std::uint8_t command, const std::vector<std::uint8_t>& data) {
  // Copy matching callbacks under lock, then invoke outside of lock.
  std::vector<ComCallback> callbacks;
  {
    std::lock_guard<std::mutex> guard(_subscriber_mutex);
    callbacks.reserve(_com_subscriptions.size());
    for (const auto& [token, entry] : _com_subscriptions) {
      if (!entry.callback)
        continue;
      if (entry.endpoints.empty()) {
        callbacks.push_back(entry.callback);
      } else {
        for (const auto& ep : entry.endpoints) {
          if (endpointMatches(ep, source)) {
            callbacks.push_back(entry.callback);
            break;
          }
        }
      }
    }
  }

  for (const auto& cb : callbacks) {
    try {
      cb(source, command, data);
    } catch (const std::exception& e) {
      // Silently absorb — listener must not crash from subscriber exceptions.
    } catch (...) {
    }
  }
}

bool ComInterface::startListener() {
  if (_listener_is_running)
    return false;

  _thread = std::make_unique<std::thread>(&ComInterface::listener, this);

  while (!_listener_is_running)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  return true;
}

void ComInterface::stopListener() {
  _shut_down_listener = true;
  if (_thread && _thread->joinable()) {
    _thread->join();
  }
}

bool ComInterface::hasError() const {
  return _communication_error;
}

} // namespace com

} // namespace sensorring

} // namespace eduart