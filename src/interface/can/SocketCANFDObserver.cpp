#include "SocketCANFDObserver.hpp"
#include <sys/time.h>

namespace edu
{

SocketCANFDObserver::SocketCANFDObserver()
{
  std::vector<canid_t> _canids;
  _seconds = 0;
  _usec = 0;
}

SocketCANFDObserver::~SocketCANFDObserver()
{

}

void SocketCANFDObserver::addCANId(const canid_t canid)
{
  _canids.push_back(canid);
}

const std::vector<canid_t>& SocketCANFDObserver::getCANIds() const
{
  return _canids;
}

void SocketCANFDObserver::forwardNotification(const canfd_frame& frame)
{
    // Take time stamp
    timeval clock;
    ::gettimeofday(&clock, 0);
    _seconds = clock.tv_sec;
    _usec = clock.tv_usec;
    
	notify(frame);
}

bool SocketCANFDObserver::checkConnectionStatus(unsigned int timeoutInMillis)
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

} // namespace
