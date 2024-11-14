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
	 * The SocketCANFD class instance reads all CAN data packets and distributes them according to the observer IDs.
	 * @param[in] observer Observer instance, which should be notified when data is available.
	 * @return success==true
	 */
	std::string getInterfaceName();

	/**
	 * The SocketCANFD class instance reads all CAN data packets and distributes them according to the observer IDs.
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
	 * Send CAN frame.
	 * @param[in] frame CAN frame
	 */
	virtual bool send(Endpoint target, const std::vector<std::uint8_t>& data) = 0;

	/**
	 * Close device file link.
	 * @return success==true
	 */
  	virtual bool openInterface(std::string interface_name) = 0;

	/**
	 * Close device file link.
	 * @return success==true
	 */
  	virtual bool closeInterface() = 0;

protected:
	virtual bool listener() = 0;

	std::atomic<bool> _listenerIsRunning;

	std::atomic<bool> _shutDownListener;

	std::string _interface_name;

	std::mutex _mutex;

private:

	std::vector<ComObserver*> _observers;

	std::unique_ptr<std::thread> _thread;

};

}; // namespace