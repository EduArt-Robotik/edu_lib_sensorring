#include "USBtingo.hpp"

#include <cstdint>
#include <exception>
#include <iomanip>
#include <limits>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>
#include <string>
#include <usbtingo/basic_bus/Message.hpp>
#include <usbtingo/can/Dlc.hpp>
#include <usbtingo/device/DeviceFactory.hpp>

#include "sensorring/logger/Logger.hpp"

using namespace eduart::sensorring::transport::protocol;

namespace eduart {

namespace sensorring {

namespace com {

USBtingo::USBtingo(std::string id)
    : ComInterface({ InterfaceType::UsbTingo, id })
    , _assembler(sensorring::transport::can::CanCodec::MAX_PAYLOAD_PER_FRAME)
    , _reassembler([this](const sensorring::transport::TransportFrame& frame) {
      ComEndpoint ep{ static_cast<Direction>(frame.direction), frame.boardAddress, frame.deviceId };
      dispatchMessage(ep, frame.command, frame.data);
    }) {
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

  _serial_str          = ss.str();
  _communication_error = false;
  return true;
}

bool USBtingo::send(ComEndpoint target, std::uint8_t command, const std::vector<uint8_t>& data) {
  auto frames = _assembler.assemble(static_cast<sensorring::transport::Direction>(target.direction), target.boardAddress, target.deviceId, command, data);

  for (const auto& frame : frames) {
    auto canFrame = sensorring::transport::can::CanCodec::encode(frame);
    if (!sendCanFrame(canFrame.id, canFrame.data, true)) {
      _communication_error = true;
      throw std::runtime_error("Unable to send message on interface " + _id.name);
    }
  }
  _communication_error = false;
  return true;
}

bool USBtingo::sendCanFrame(std::uint32_t can_id, const std::vector<uint8_t>& data, bool fd) {
  (void)fd;
  usbtingo::bus::Message msg(can_id, data);
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
            std::size_t frameLen = usbtingo::can::Dlc::dlc_to_bytes(rx_frame.dlc);
            if (frameLen < HEADER_SIZE)
              continue;

            std::uint8_t sysId = (rx_frame.id >> 8) & 0x07;
            if (sysId != sensorring::transport::can::CanCodec::SYSID_SENSOR_RING)
              continue;

            auto transportFrame = sensorring::transport::can::CanCodec::decode(rx_frame.id, rx_frame.data.data(), frameLen);
            _reassembler.processFrame(transportFrame);
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

} // namespace sensorring

} // namespace eduart