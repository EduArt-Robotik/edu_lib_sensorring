#include "ComInterface.hpp"
#include <iostream>

namespace eduart{

namespace com{

ComInterface::ComInterface(std::string interface_name) : _interface_name(interface_name){
    
	_listenerIsRunning = false;
	_shutDownListener = false;
}

ComInterface::~ComInterface(){
    if(!_listenerIsRunning) stopListener();
}

std::string ComInterface::getInterfaceName()
{
	return _interface_name;
}

bool ComInterface::registerObserver(ComObserver* observer)
{
	_mutex.lock();
	if(observer) _observers.push_back(observer);
	_mutex.unlock();

	return true;
}

void ComInterface::clearObservers()
{
	_mutex.lock();
	_observers.clear();
	_mutex.unlock();
}

bool ComInterface::startListener()
{
	if(_listenerIsRunning) return false;

	_thread = std::make_unique<std::thread>(&ComInterface::listener, this);
	
	while(!_listenerIsRunning)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	return true;
}

void ComInterface::stopListener()
{
	if(!_listenerIsRunning) return;
	
	_shutDownListener = true;
	if(_thread->joinable())
		_thread->join();
	
}

const std::vector<ComEndpoint>& ComInterface::getEndpoints(){
	return _endpoints;
}

}

}