#include "ComObserver.hpp"

namespace com{

ComObserver::ComObserver()
{
  std::vector<Endpoint> _canids;
  _seconds = 0;
  _usec = 0;
}

ComObserver::~ComObserver()
{

}

void ComObserver::addEndpoint(const Endpoint canid)
{
  _canids.push_back(canid);
}

const std::vector<Endpoint>& ComObserver::getEndpoints() const
{
  return _canids;
}

void ComObserver::forwardNotification(const Endpoint source, const std::vector<uint8_t>& data)
{
    // Take time stamp
    timeval clock;
    ::gettimeofday(&clock, 0);
    _seconds = clock.tv_sec;
    _usec = clock.tv_usec;
    
	notify(const Endpoint source, const std::vector<uint8_t>& data);
}

bool ComObserver::checkConnectionStatus(unsigned int timeoutInMillis)
{
      // Take time stamp
    timeval clock;
    ::gettimeofday(&clock, 0);
    long seconds = clock.tv_sec;
    long usec = clock.tv_usec;

    long deltaSec  = seconds - _seconds;
    long deltaUSec = usec - _usec;

    long elapsed = deltaSec * 1000 + deltaUSec / 1000;
    return elapsed < timeoutInMillis;
}

}; // namespace