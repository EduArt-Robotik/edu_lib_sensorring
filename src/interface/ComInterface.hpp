#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "ComEndpoints.hpp"
#include "ComObserver.hpp"

namespace eduart {

namespace com {

class ComInterface {

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
   * Register a ComObserver with the ComInterface. The observer gets notified on all future incoming messages.
   * @param[in] observer Observer instance, which should be notified when data is available.
   * @return success==true
   */
  bool unregisterObserver(ComObserver* observer);

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

  /**
   * Add endpoint for a new tof sensor
   * @param[in] idx index of the sensor
   */
  virtual void addToFSensorToEndpointMap(std::size_t idx) = 0;

  /**
   * Add endpoint for a new thermal sensor
   * @param[in] idx index of the sensor
   */
  virtual void addThermalSensorToEndpointMap(std::size_t idx) = 0;

protected:
  using LockGuard = std::lock_guard<std::mutex>;

  virtual bool listener() = 0;

  std::atomic<bool> _listenerIsRunning;

  std::atomic<bool> _shutDownListener;

  std::string _interface_name;

  std::mutex _mutex;

  std::vector<ComEndpoint> _endpoints;

  std::set<ComObserver*> _observers;

private:
  std::unique_ptr<std::thread> _thread;
};

} // namespace com

} // namespace eduart