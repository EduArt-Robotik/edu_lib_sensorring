#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <map>

#include <usbtingo/basic_bus/BasicBus.hpp>

#include "interface/ComInterface.hpp"
#include "interface/ComObserver.hpp"

namespace eduart{

namespace com{

/**
 * @class USBtingo
 * @brief CAN communication class using the USBtingo USB to CANFD converter.
 * @author Hannes Duske
 * @date 29.01.2025
 */
class USBtingo : public ComInterface
{
public:
  /**
   * Constructor
   * @param[in] serial serial number of the USBtingo to use
   */
  USBtingo(std::string serial);

  /**
   * Destructor
   */
  ~USBtingo();

  /**
   * Open CAN interface.
   * @param[in] interface_name CAN interface name specified with slcand.
   * @return success==true
   */
  bool openInterface(std::string interface_name) override;

  /**
   * Send generic communication messsage.
   * @param[in] target ComEndpoint to which the message is sent.
   * @param[in] data Message payload.
   * @return success==true
   */
  bool send(ComEndpoint target, const std::vector<uint8_t>& data) override;

  /**
   * Close device file link.
   * @return success==true
   */
  bool closeInterface() override;

  /**
   * Add endpoint for a new tof sensor
   * @param[in] idx index of the sensor
   */
  void addToFSensorToEndpointMap(std::size_t idx) override;

  /**
   * Add endpoint for a new thermal sensor
   * @param[in] idx index of the sensor
   */
  void addThermalSensorToEndpointMap(std::size_t idx) override;
private:

  void fillEndpointMap();

  std::uint32_t mapEndpointToId(ComEndpoint ep);
  
  ComEndpoint mapIdToEndpoint(std::uint32_t id);

  std::map<ComEndpoint, std::uint32_t> _id_map;

  bool listener() override;

  std::unique_ptr<usbtingo::device::Device> _dev;
};

}

}