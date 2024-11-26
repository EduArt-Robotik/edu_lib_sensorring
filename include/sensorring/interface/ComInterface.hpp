#pragma once

#include <vector>
#include <cstdint>
#include <thread>
#include <string>
#include <atomic>

#include "ComEndpoints.hpp"
#include "ComObserver.hpp"
#include "SingletonTemplate.hpp"

namespace com{

class ComInterface : public Singleton<ComInterface>{
public:

	/**
	 * Constructor
	 * @param[in] std::string name of the interface to be opened
	 */
	ComInterface(std::string interface_name);

	/**
	 * Destructor
	 */
	~ComInterface();

	/**
	 * Get the interface name of the ComInterface object
	 * @return name of the interface
	 */
	std::string getInterfaceName();

	/**
	 * Get all known ComEndpoints.
	 * @return Vector of all known ComEndpoints. Messages may only be sent to one of the known endpoints.
	 */
	const std::vector<ComEndpoint>& getEndpoints();

	/**
	 * Register a ComObserver with the ComInterface. The observer gets notified on all future incoming messages.
	 * @param[in] observer Observer instance, which should be notified when data is available.
	 * @return success==true
	 */
	bool registerObserver(ComObserver* observer);

	/**
	 * Remove all registered observers
	 */
	void clearObservers();

	/**
	 * Start listener thread.
	 * @return success==true, failure==false (e.g. when listener is already running)
	 */
	bool startListener();

	/**
	 * Terminate listener thread
	 */
  	void stopListener();

	/**
	 * Send a generic communication message.
	 * @param[in] target ComEndpoint to which the message is sent.
	 * @param[in] data Vector holding the message payload.
	 */
	virtual bool send(ComEndpoint target, const std::vector<std::uint8_t>& data) = 0;

	/**
	 * Open the communication interface.
	 * @return success==true
	 */
  	virtual bool openInterface(std::string interface_name) = 0;

	/**
	 * Close the communication interface.
	 * @return success==true
	 */
  	virtual bool closeInterface() = 0;

protected:
	virtual bool listener() = 0;

	std::atomic<bool> _listenerIsRunning;

	std::atomic<bool> _shutDownListener;

	std::string _interface_name;

	std::mutex _mutex;

	std::vector<ComEndpoint> _endpoints;

	std::vector<ComObserver*> _observers;

private:

	std::unique_ptr<std::thread> _thread;

};

};