#include "ComInterface.hpp"

namespace eduart {

namespace com {

ComInterface::ComInterface()
    : _communication_error(false)
    , _listener_is_running(false)
    , _shut_down_listener(false)
    , _interface_name("")
    , _thread{nullptr} {
}

ComInterface::~ComInterface() {
  stopListener();
}

std::string ComInterface::getInterfaceName() const {
  return _interface_name;
}

bool ComInterface::registerObserver(ComObserver* observer) {
  LockGuard guard(_mutex);
  if (observer) {
    auto result = _observers.insert(observer);

    if (result.second) {
      return true;
    }
  }
  return false;
}

bool ComInterface::unregisterObserver(ComObserver* observer) {
  LockGuard guard(_mutex);
  if (observer) {
    auto result = _observers.erase(observer);

    if (result > 0) {
      return true;
    }
  }
  return false;
}

void ComInterface::clearObservers() {
  LockGuard guard(_mutex);
  _observers.clear();
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

const std::set<ComEndpoint>& ComInterface::getEndpoints() const {
  return _endpoints;
}

bool ComInterface::hasError() const {
  return _communication_error;
}

} // namespace com

} // namespace eduart