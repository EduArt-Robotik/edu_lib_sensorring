#include "USBtingo.hpp"

#include <cstdint>
#include <exception>
#include <iomanip>
#include <limits>
#include <string>
#include <usbtingo/basic_bus/Message.hpp>
#include <usbtingo/can/Dlc.hpp>
#include <usbtingo/device/DeviceFactory.hpp>

#include "sensorring/logger/Logger.hpp"

#include "CanEndpointMap.hpp"
#include "canprotocol.hpp"

namespace eduart {

namespace com {

USBtingo::USBtingo(std::string id)
    : ComInterface({ InterfaceType::UsbTingo, id }) {
  if (!openInterface()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open interface: " + _id.name);
  }

  startListener();
}

USBtingo::~USBtingo() {
  stopListener();
  closeInterface();
}

bool USBtingo::openInterface() {
  std::uint32_t serial_number = 0;
  try {
    // if the serial number is specified in hex Format
    if (_id.name.find("0x", 0) == 0) {
      // serial number is specified in decimal format
      serial_number = static_cast<std::uint32_t>(std::stoul(_id.name.substr(2), nullptr, 16));
    } else {
      // serial number is specified in decimal format
      serial_number = static_cast<std::uint32_t>(std::stoul(_id.name));
    }
  } catch (...) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "The USBtingo interface name must be an unsigned integer but got \"" + _id.name + "\" instead.");
    return false;
  }

  _dev = usbtingo::device::DeviceFactory::create(serial_number);

  if (!_dev) {
    if (serial_number == 0) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "No USBtingo connected.");
    }
    return false;
  }
  if (!_dev->is_alive())
    return false;
  if (!_dev->set_mode(usbtingo::device::Mode::OFF))
    return false;
  if (!_dev->set_baudrate(1000000, 5000000))
    return false;
  if (!_dev->set_protocol(usbtingo::device::Protocol::CAN_FD, 0b00010001))
    return false;
  if (!_dev->set_mode(usbtingo::device::Mode::ACTIVE))
    return false;

  std::stringstream ss;
  ss << "0x" << std::hex << std::nouppercase << std::setw(8) << std::setfill('0') << _dev->get_serial();

  _id.name             = ss.str();
  _communication_error = false;
  return true;
}

bool USBtingo::send(ComEndpoint target, const std::vector<uint8_t>& data) {
  usbtingo::bus::Message msg(CanEndpointMap::getInstance()->mapEndpointToId(target), data);
  if (!_dev->send_can(msg.to_CanTxFrame(true))) {
    _communication_error = true;
    throw std::runtime_error("Unable to send message on interface " + _id.name);
  }
  _communication_error = false;
  return true;
}

bool USBtingo::listener() {
  if (!_dev || !_dev->is_alive()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Error starting listener on interface " + _id.name + ". Interface not initialized.");
    return false;
  }

  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Starting listener on interface " + _id.name);

  std::vector<usbtingo::device::CanRxFrame> rx_frames;
  std::vector<usbtingo::device::TxEventFrame> tx_event_frames;

  auto zero_timeout = std::chrono::microseconds(0);
  auto can_future   = _dev->request_can_async();

  _shut_down_listener  = false;
  _listener_is_running = true;
  while (!_shut_down_listener) {
    {
      LockGuard guard(_mutex);

      // can message handling
      if (can_future.valid() && can_future.wait_for(zero_timeout) == std::future_status::ready) {
        if (can_future.get()) {
          _dev->receive_can_async(rx_frames, tx_event_frames);

          // forward can frames
          for (const auto& rx_frame : rx_frames) {

            try {
              auto endpoint = CanEndpointMap::getInstance()->mapIdToEndpoint(rx_frame.id);
              dispatchMessage(endpoint, std::vector<std::uint8_t>(rx_frame.data.begin(), rx_frame.data.begin() + usbtingo::can::Dlc::dlc_to_bytes(rx_frame.dlc)));
            } catch (const std::exception&) {
              logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Tried to map unknown CAN ID on interface " + _id.name);
            }
          }
          rx_frames.clear();
          tx_event_frames.clear();
        }
        can_future = _dev->request_can_async();
      }
    }

    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Stopping can listener on interface " + _id.name);

  _dev->cancel_async_can_request();
  _listener_is_running = false;
  return true;
}

bool USBtingo::closeInterface() {
  return true;
}

bool USBtingo::repairInterface() {
  stopListener();
  closeInterface();

  if (openInterface()) {
    if (startListener())
      return true;
  }
  return false;
}

} // namespace com

} // namespace eduart