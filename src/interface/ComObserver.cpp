#include "ComObserver.hpp"

namespace com{

ComObserver::ComObserver()
{

}

ComObserver::~ComObserver()
{

}

void ComObserver::addEndpoint(const Endpoint canid)
{
  _endpoints.push_back(canid);
}

const std::vector<Endpoint>& ComObserver::getEndpoints() const
{
  return _endpoints;
}

void ComObserver::forwardNotification(const Endpoint source, const std::vector<uint8_t>& data)
{
    // Take time stamp
    _timestamp = std::chrono::steady_clock::now();
    
    notify(source, data);
}

bool ComObserver::checkConnectionStatus(unsigned int timeoutInMillis)
{
    // Take time stamp
    auto elapsed = std::chrono::steady_clock::now() - _timestamp;
   
    return elapsed < std::chrono::milliseconds(timeoutInMillis);
}

}; // namespace