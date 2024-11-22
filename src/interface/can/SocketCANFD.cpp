#include "SocketCANFD.hpp"
#include "canprotocol.hpp"
#include "Logger.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <algorithm>
#include <chrono>

namespace com
{

SocketCANFD::SocketCANFD(std::string interface_name, std::size_t sensor_count) : ComInterface(interface_name), _soc(0){

	if(!openInterface(interface_name)){
		Logger::getInstance()->log(LogVerbosity::Error, std::stringstream() << "Cannot open interface: " << interface_name);
	}

	_endpoints = ComEndpoint::createEndpoints(sensor_count);
	fillMap(sensor_count);
	startListener();
}

SocketCANFD::~SocketCANFD(){
	stopListener();
	closeInterface();
}

bool SocketCANFD::openInterface(std::string interface_name)
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
	strcpy(ifr.ifr_name, interface_name.c_str());

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

bool SocketCANFD::send(ComEndpoint target, const std::vector<uint8_t>& data){

	canid_t id = mapEndpointToId(target);
	return send(id, data);
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
			Logger::getInstance()->log(LogVerbosity::Error, std::stringstream() << "Can transmission error for command " << (int)(frame->data[0]) << ", returned " << retval << " submitted bytes instead of " << sizeof(canfd_frame));
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

bool SocketCANFD::listener()
{
	_shutDownListener  = false;

	canfd_frame frame_rd;
	int recvbytes = 0;

	timeval timeout = {0, 100};
	fd_set readSet;

	Logger::getInstance()->log(LogVerbosity::Debug, std::stringstream() << "Starting can listener on interface " << _interface_name);

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
					for(auto observer : _observers)
					{
						if (observer)
						{
							for(auto endpoint : observer->getEndpoints()){
								canid_t canid = mapEndpointToId(endpoint);
								if(canid == frame_rd.can_id)
								observer->forwardNotification(endpoint, std::vector<std::uint8_t>(frame_rd.data, frame_rd.data + frame_rd.len));
							}
						}
					}
				}
			}
		}
		_mutex.unlock();

		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	Logger::getInstance()->log(LogVerbosity::Debug, std::stringstream()<< "Stopping can listener on interface " << _interface_name);

	_listenerIsRunning = false;
	return true;
}

bool SocketCANFD::closeInterface()
{
	bool retval = false;
	if(_soc)
	{
		retval = (close(_soc)==0);
	}
	return retval;
}

void SocketCANFD::fillMap(std::size_t sensor_count){
	canid_t canid_tof_status, canid_tof_request, canid_tof_data_in, canid_tof_data_out, canid_broadcast;
	CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status,  canid_tof_request,  canid_broadcast);
	CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_DATA,   canid_tof_data_in, canid_tof_data_out, canid_broadcast);

	canid_t canid_thermal_status, canid_thermal_request, canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast;
	CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_STATUS, canid_thermal_status,  canid_thermal_request,  canid_thermal_broadcast);
	CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_DATA,   canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast);

	canid_t canid_light_in, canid_light_out, canid_light;
	CanProtocol::makeCanStdID(SYSID_LIGHT, NODEID_HEADLEFT, canid_light_in, canid_light_out, canid_light);

	_id_map[ComEndpoint("tof_status")] 		= canid_tof_status;
    _id_map[ComEndpoint("tof_request")]		= canid_tof_request;
    _id_map[ComEndpoint("thermal_status")]	= canid_thermal_status;
    _id_map[ComEndpoint("thermal_request")]	= canid_thermal_request;
    _id_map[ComEndpoint("light")]			= canid_light;
    _id_map[ComEndpoint("broadcast")]		= canid_broadcast;

	for(std::size_t idx=0; idx<sensor_count; idx++){
		_id_map[ComEndpoint("tof" + std::to_string(idx) + "_data")] = canid_tof_data_in + idx;
		_id_map[ComEndpoint("thermal" + std::to_string(idx) + "_data")] = canid_thermal_data_in + idx;

	}
}

canid_t SocketCANFD::mapEndpointToId(ComEndpoint endpoint){
	canid_t id = _id_map.at(endpoint); // may throw out_of_range exception
	return id;
};
  
// ComEndpoint SocketCANFD::mapIdToEndpoint(canid_t id){

// };

} // namespace
