#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/interface/ComObserver.hpp"

namespace eduart {

namespace com {

class ComInterface {

public:
  /**
   * Constructor
   */
  ComInterface(ComInterfaceID id);

  /**
   * Destructor
   */
  virtual ~ComInterface();

  /**
   * Get the ComInterfaceID of the ComInterface object
   * @return ComInterfaceID of the interface
   */
  ComInterfaceID getID() const;

  /**
   * Get all known ComEndpoints.
   * @return Set of all known ComEndpoints. Messages may only be sent to one of the known endpoints.
   */
  const std::unordered_set<ComEndpoint>& getEndpoints() const;

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
  virtual bool openInterface() = 0;

  /**
   * Close the communication interface.
   * @return success==true
   */
  virtual bool closeInterface() = 0;

  /**
   * Repair the connection in case of an error.
   * @return success==true
   */
  virtual bool repairInterface() = 0;

  /**
   * Check if a communication error has occurred.
   * @return error==true
   */
  bool hasError() const;

  /**
   * Add endpoint for a new sensor board
   */
  virtual void addSensorBoardEndpoint() = 0;

  /**
   * Add endpoint for a new tof sensor
   * @param[in] idx index of the sensor
   */
  virtual void addTofSensorEndpoint(std::size_t idx) = 0;

  /**
   * Add endpoint for a new thermal sensor
   * @param[in] idx index of the sensor
   */
  virtual void addThermalSensorEndpoint(std::size_t idx) = 0;

  /**
   * Add endpoint for a new light sensor
   */
  virtual void addLightSensorEndpoint() = 0;

protected:
  using LockGuard = std::lock_guard<std::mutex>;

  virtual bool listener() = 0;

  std::atomic<bool> _communication_error;

  std::atomic<bool> _listener_is_running;

  std::atomic<bool> _shut_down_listener;

  std::mutex _mutex;

  std::unordered_set<ComObserver*> _observers;

  ComInterfaceID _id;

private:
  std::unique_ptr<std::thread> _thread;
};

} // namespace com

} // namespace eduart