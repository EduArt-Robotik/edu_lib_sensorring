#include "ComInterface.hpp"
#include <iostream>

namespace com{

ComInterface::ComInterface(std::string interface_name) : _interface_name(interface_name){
    
	_listenerIsRunning = false;
	_shutDownListener = false;
	
	if(!openInterface(interface_name)){
		std::cout << "WARNING: Cannot open interface: " << interface_name << std::endl;
	}
}

ComInterface::~ComInterface(){
    if(!_listenerIsRunning) stopListener();
	closeInterface();
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
	_shutDownListener = true;

	_thread->join();

	_thread.release();
}

}; // namespace