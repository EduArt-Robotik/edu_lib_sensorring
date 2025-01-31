#pragma once

#include "ComInterface.hpp"
#include "ComObserver.hpp"

#include <usbtingo/basic_bus/BasicBus.hpp>

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <map>


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
   * @param[in] sensor_count number of sensor boards that are connected on this interface
   */
  USBtingo(std::string serial, std::size_t sensor_count);

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

private:

  void fillMap(std::size_t sensor_count);

  std::uint32_t mapEndpointToId(ComEndpoint endpoint);
  
  //ComEndpoint mapIdToEndpoint(std::uint32_t id);

  std::map<ComEndpoint, std::uint32_t> _id_map;

  bool listener() override;

  std::unique_ptr<usbtingo::device::Device> _dev;
};

}

}