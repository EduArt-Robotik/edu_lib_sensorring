#include "USBtingo.hpp"

#include <cstdint>
#include <string>
#include <usbtingo/basic_bus/Message.hpp>
#include <usbtingo/can/Dlc.hpp>
#include <usbtingo/device/DeviceFactory.hpp>

#include "logger/Logger.hpp"

#include "canprotocol.hpp"

namespace eduart {

namespace com {

USBtingo::USBtingo(std::string interface_name)
    : ComInterface(interface_name) {
  if (!openInterface(interface_name)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Cannot open interface: " + interface_name);
  }

  _endpoints = ComEndpoint::createEndpoints();
  fillEndpointMap();
  startListener();
}

USBtingo::~USBtingo() {
  stopListener();
  closeInterface();
}

bool USBtingo::openInterface(std::string interface_name) {
  std::uint32_t serial_number = 0;
  try {
    // if the serial number is specified in hex Format
    if (interface_name.find("0x", 0) == 0) {
      // serial number is specified in decimal format
      serial_number = static_cast<std::uint32_t>(std::stoul(interface_name.substr(2), nullptr, 16));
    } else {
      // serial number is specified in decimal format
      serial_number = static_cast<std::uint32_t>(std::stoul(interface_name));
    }
  } catch (...) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid interface name for interface type USBtingo. The interface name must be an unsigned integer." + interface_name);
    return false;
  }

  _dev = usbtingo::device::DeviceFactory::create(serial_number);

  if (!_dev)
    return false;
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

  return true;
}

bool USBtingo::send(ComEndpoint target, const std::vector<uint8_t>& data) {
  usbtingo::bus::Message msg(mapEndpointToId(target), data);
  return _dev->send_can(msg.to_CanTxFrame(true));
}

bool USBtingo::listener() {
  if (!_dev || !_dev->is_alive()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Error starting can listener on interface " + _interface_name + ". Interface not initialized.");
    return false;
  }

  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Starting can listener on interface " + _interface_name);

  std::vector<usbtingo::device::CanRxFrame> rx_frames;
  std::vector<usbtingo::device::TxEventFrame> tx_event_frames;

  auto zero_timeout = std::chrono::microseconds(0);
  auto can_future   = _dev->request_can_async();

  _shutDownListener  = false;
  _listenerIsRunning = true;
  while (!_shutDownListener) {
    {
      LockGuard guard(_mutex);

      // can message handling
      if (can_future.valid() && can_future.wait_for(zero_timeout) == std::future_status::ready) {
        if (can_future.get()) {
          _dev->receive_can_async(rx_frames, tx_event_frames);

          // forward can frames
          for (const auto& rx_frame : rx_frames) {

            auto endpoint = mapIdToEndpoint(rx_frame.id);
            for (auto observer : _observers) {
              if (observer)
                observer->forwardNotification(endpoint, std::vector<std::uint8_t>(rx_frame.data.begin(), rx_frame.data.begin() + usbtingo::can::Dlc::dlc_to_bytes(rx_frame.dlc)));
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

  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Stopping can listener on interface " + _interface_name);

  _dev->cancel_async_can_request();
  _listenerIsRunning = false;
  return true;
}

bool USBtingo::closeInterface() {
  _dev.release();
  return true;
}

void USBtingo::fillEndpointMap() {
  std::uint32_t canid_tof_status, canid_tof_request, canid_broadcast;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast);

  std::uint32_t canid_thermal_status, canid_thermal_request, canid_thermal_broadcast;
  CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_STATUS, canid_thermal_status, canid_thermal_request, canid_thermal_broadcast);

  std::uint32_t canid_light_in, canid_light_out, canid_light;
  CanProtocol::makeCanStdID(SYSID_LIGHT, NODEID_HEADLEFT, canid_light_in, canid_light_out, canid_light);

  _id_map[ComEndpoint("tof_status")]      = canid_tof_status;
  _id_map[ComEndpoint("tof_request")]     = canid_tof_request;
  _id_map[ComEndpoint("thermal_status")]  = canid_thermal_status;
  _id_map[ComEndpoint("thermal_request")] = canid_thermal_request;
  _id_map[ComEndpoint("light")]           = canid_light;
  _id_map[ComEndpoint("broadcast")]       = canid_broadcast;
}

void USBtingo::addToFSensorToEndpointMap(std::size_t idx) {
  canid_t canid_tof_data_in, canid_tof_data_out, canid_broadcast;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_DATA, canid_tof_data_in, canid_tof_data_out, canid_broadcast);

  _id_map[ComEndpoint("tof" + std::to_string(idx) + "_data")] = canid_tof_data_in + idx;
}

void USBtingo::addThermalSensorToEndpointMap(std::size_t idx) {
  canid_t canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast;
  CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_DATA, canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast);

  _id_map[ComEndpoint("thermal" + std::to_string(idx) + "_data")] = canid_thermal_data_in + idx;
}

std::uint32_t USBtingo::mapEndpointToId(ComEndpoint ep) {
  std::uint32_t id = _id_map.at(ep); // may throw out_of_range exception
  return id;
}

ComEndpoint USBtingo::mapIdToEndpoint(std::uint32_t id) {
  auto it = std::find_if(_id_map.begin(), _id_map.end(), [&id](const auto& pair) { return pair.second == id; });

  if (it != _id_map.end()) {
    return it->first;
  } else {
    throw std::runtime_error("No Endpoint found for given CAN ID");
  }
}

}

}