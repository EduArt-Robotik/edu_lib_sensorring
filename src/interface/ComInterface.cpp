#include "ComInterface.hpp"

#include <algorithm>

namespace eduart {

namespace com {

ComInterface::ComInterface(std::string interface_name)
    : _listenerIsRunning(false)
    , _shutDownListener(false)
    , _interface_name(interface_name) {
}

ComInterface::~ComInterface() {
  stopListener();
}

std::string ComInterface::getInterfaceName() {
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
  if (_listenerIsRunning)
    return false;

  _thread = std::make_unique<std::thread>(&ComInterface::listener, this);

  while (!_listenerIsRunning)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  return true;
}

void ComInterface::stopListener() {
  if (!_listenerIsRunning)
    return;

  _shutDownListener = true;
  while (!_listenerIsRunning) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  if (_thread->joinable()) {
    _thread->join();
  }
}

const std::set<ComEndpoint>& ComInterface::getEndpoints() {
  return _endpoints;
}

} // namespace com

} // namespace eduart