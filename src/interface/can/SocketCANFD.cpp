#include "can/SocketCANFD.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <algorithm>

namespace edu
{

SocketCANFD::SocketCANFD(std::string devFile) : _devFile(devFile)
{
	std::vector<SocketCANFDObserver*> _observers;
	
	_soc = 0;
	_listenerIsRunning = false;
	_shutDownListener  = false;

	if(!openPort(devFile.c_str()))
		std::cout << "WARNING: Cannot open CAN device interface: " << devFile << std::endl;
}

SocketCANFD::~SocketCANFD()
{
	if(!_listenerIsRunning) stopListener();
	closePort();
}

std::string SocketCANFD::getInterfaceName()
{
	return _devFile;
}

bool SocketCANFD::registerObserver(SocketCANFDObserver* observer)
{
	_mutex.lock();
	if(observer) _observers.push_back(observer);
	_mutex.unlock();

	return true;
}

void SocketCANFD::clearObservers()
{
	_mutex.lock();
	_observers.clear();
	_mutex.unlock();
}

bool SocketCANFD::openPort(const char *port)
{
	ifreq ifr;
	sockaddr_can addr;

	_soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(_soc < 0)
	{
		return false;
	}

	int canfd_enable = 1;
	if (setsockopt(_soc, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_enable, sizeof(canfd_enable)) < 0) {
				perror("Cannot set CAN FD frames");
				return -1;
		}

	addr.can_family = AF_CAN;
	strcpy(ifr.ifr_name, port);

	if (ioctl(_soc, SIOCGIFINDEX, &ifr) < 0)
	{
		return false;
	}

	addr.can_ifindex = ifr.ifr_ifindex;

	fcntl(_soc, F_SETFL, O_NONBLOCK);

	if (bind(_soc, (sockaddr *)&addr, sizeof(addr)) < 0)
	{
		return false;
	}

	return true;
}

bool SocketCANFD::send(canid_t canid, const std::vector<uint8_t>& tx_buf){

	if(tx_buf.size() <= CANFD_MAX_DLEN){
		auto frame = std::make_unique<canfd_frame>();
		frame->can_id = canid;
		frame->len = tx_buf.size();

		std::copy_n(tx_buf.begin(), tx_buf.size(), frame->data);
		return send(frame.get());
	}

	return false;
};

bool SocketCANFD::send(const canfd_frame* frame)
{
	if(frame){
		static double _timeCom = 0.0;
		timeval clock;
		double now = 0.0;
		do{
				::gettimeofday(&clock, 0);
				now = static_cast<double>(clock.tv_sec) + static_cast<double>(clock.tv_usec) * 1.0e-6;
		}while((now - _timeCom) < 0.002);
		_timeCom = now;
			
		int retval;
		_mutex.lock();
		retval = write(_soc, frame, sizeof(canfd_frame));
		_mutex.unlock();
		
		if (retval != sizeof(canfd_frame)){
			std::cout << "Can transmission error for command " << (int)(frame->data[0]) << ", returned " << retval << " submitted bytes instead of " << sizeof(canfd_frame) << std::endl;
			return false;
		}
		else{
			return true;
		}
	}
	else{
		return false;
	}
}

bool SocketCANFD::startListener()
{
	if(_listenerIsRunning) return false;

	_thread = std::make_unique<std::thread>(&SocketCANFD::listener, this);

	while(!_listenerIsRunning)
		usleep(100);

	return true;
}

bool SocketCANFD::listener()
{
	_shutDownListener  = false;

	canfd_frame frame_rd;
	int recvbytes = 0;

	timeval timeout = {0, 100};
	fd_set readSet;

	std::cout << "# Listener start" << std::endl;

	_listenerIsRunning = true;
	while(!_shutDownListener)
	{
		FD_ZERO(&readSet);

		_mutex.lock();
		FD_SET(_soc, &readSet);
		if (select((_soc + 1), &readSet, NULL, NULL, &timeout) >= 0)
		{
			if (FD_ISSET(_soc, &readSet))
			{
				recvbytes = read(_soc, &frame_rd, sizeof(canfd_frame));
				if(recvbytes)
				{
					for(auto observer : _observers) // check all observers
					{
						if (observer)
						{
							for(auto canid : observer->getCANIds()){ // Why error??? -> getCANIds return uninitialized vector :(
								if(canid == frame_rd.can_id)
								observer->forwardNotification(frame_rd);
							}
						}
					}
				}
			}
		}
		_mutex.unlock();

		usleep(100);
	}
	std::cout << "# Listener stop" << std::endl;

	_listenerIsRunning = false;
	return true;
}

void SocketCANFD::stopListener()
{
	_shutDownListener = true;

	_thread->join();

	_thread.release();
}

bool SocketCANFD::closePort()
{
	bool retval = false;
	if(_soc)
	{
		retval = (close(_soc)==0);
	}
	return retval;
}

} // namespace
