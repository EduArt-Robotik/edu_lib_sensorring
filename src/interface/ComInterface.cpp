#include "ComInterface.hpp"

namespace com{

ComInterface::ComInterface(std::string interface_name) : _interface_name(interface_name){

}

ComInterface::~ComInterface(){
    if(!_listenerIsRunning) stopListener();
}

std::string ComInterface::getInterfaceName()
{
	return _devFile;
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
		usleep(100);

	return true;
}

void ComInterface::stopListener()
{
	_shutDownListener = true;

	_thread->join();

	_thread.release();
}

}; // namespace