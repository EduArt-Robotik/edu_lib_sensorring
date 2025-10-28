#include "ComObserver.hpp"

namespace eduart {

namespace com {

ComObserver::ComObserver() {
}

ComObserver::~ComObserver() {
}

bool ComObserver::addEndpoint(const ComEndpoint canid) {
  auto result = _endpoints.insert(canid);
  if (result.second) {
    return true;
  }
  return false;
}

bool ComObserver::removeEndpoint(const ComEndpoint canid) {
  auto result = _endpoints.erase(canid);
  if (result > 0) {
    return true;
  }
  return false;
}

const std::set<ComEndpoint>& ComObserver::getEndpoints() const {
  return _endpoints;
}

void ComObserver::forwardNotification(const ComEndpoint source, const std::vector<uint8_t>& data) {
  for (const auto& endpoint : _endpoints) {
    if (endpoint == source) {
      // Take time stamp
      _timestamp = std::chrono::steady_clock::now();

      // Trigger callback
      notify(source, data);
      break;
    }
  }
}

bool ComObserver::checkConnectionStatus(unsigned int timeoutInMillis) {
  // Take time stamp
  auto elapsed = std::chrono::steady_clock::now() - _timestamp;

  return elapsed < std::chrono::milliseconds(timeoutInMillis);
}

} // namespace com

} // namespace eduart