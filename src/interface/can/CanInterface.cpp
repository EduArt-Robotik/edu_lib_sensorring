#include "CanInterface.hpp"

#include <sensorring_transport/ByteOperations.hpp>
#include <sensorring_transport/Protocol.hpp>

#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensorring {

namespace com {

using namespace eduart::sensorring::transport;
using namespace eduart::sensorring::transport::protocol;
using namespace eduart::sensorring::transport::protocol::sensor_board;

// ToDo: Currently parameters are set on all boards via broadcast but fetched only from the first connected board. Clean solution would be to check all boards.

CanInterface::CanInterface(ComInterfaceID id)
    : ComInterface{ id }
    , _params() {

  _com_subscription = this->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
  },
      { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } });

  // configure(); //Does not work here because we try to send data but interface is not fully initialized yet.
}

CanInterface::CanInterface(ComInterfaceID id, const CanParams& params)
    : ComInterface{ id }
    , _params(params) {

  _com_subscription = this->subscribe(
      [this](const com::ComEndpoint& source, std::uint8_t command, const std::vector<uint8_t>& data) {
        this->comCallback(source, command, data);
  },
      { com::ComEndpoint{ com::Direction::Output, com::ComEndpoint::ANY_BOARD, devbyte::BOARD } });

  // configure(); //Does not work here because we try to send data but interface is not fully initialized yet.
}

bool CanInterface::configure() {
  return setParams(_params);
}

bool CanInterface::setParams(const CanParams& params) {

  if (!setDataRate(params.data_bitrate))
    return false;

  if (!setDataSamplePoint(params.data_sample_point))
    return false;

  if (!setBrs(params.respond_with_brs)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Trying fallback with SocketCAN BRS disabled on interface " + _id.name + ".");
    if (!setBrs(false)) {
      return false;
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Setting SocketCAN BRS enable on interface " + _id.name + " failed. Restored communication with BRS turned off.");
    }
  }
  return true;
}

bool CanInterface::getParams(CanParams& params) {

  if (!getDataRate(params.data_bitrate))
    return false;

  if (!getDataSamplePoint(params.data_sample_point))
    return false;

  if (!getBrs(params.respond_with_brs))
    return false;

  params = _params;
  return true;
}

bool CanInterface::setBrs(bool enable) {
  send(com::ComEndpoint{ com::Direction::Input, com::ComEndpoint::BROADCAST, devbyte::BOARD }, PARAMETER_CANFD_SET_BRS, { static_cast<std::uint8_t>(enable) });
  std::this_thread::sleep_for(SET_PARAMETER_DELAY); // MCU may need to restart CAN controller after changing BRS setting, wait a bit before trying to fetch the parameter again.

  bool currentBrs = false;
  bool success    = getBrs(currentBrs);
  success &= (currentBrs == enable);

  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set SocketCAN BRS enable on interface " + _id.name + ": " + (currentBrs ? "true" : "false"));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to set SocketCAN BRS on interface " + _id.name);
    return false;
  }

  return success;
}

bool CanInterface::getBrs(bool& enable) {
  _got_update = false;

  send(com::ComEndpoint{ com::Direction::Input, FIRST_BOARD_ADDRESS, devbyte::BOARD }, PARAMETER_CANFD_GET_BRS, {});

  auto now = std::chrono::steady_clock::now();
  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(GET_PARAMETER_SLEEP);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got SocketCAN BRS enable update on interface " + _id.name + ": " + (_params.respond_with_brs ? "true" : "false"));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get SocketCAN BRS enable on interface " + _id.name);
    return false;
  }

  enable = _params.respond_with_brs;
  return true;
}

bool CanInterface::setDataRate(unsigned int data_rate) {
  if (!(data_rate == 0) && (data_rate < 1000000 || data_rate > 8000000)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid SocketCAN data rate value on interface " + _id.name + ": " + std::to_string(data_rate) + ". Must be between 1000000 and 8000000.");
    return false;
  }

  send(com::ComEndpoint{ com::Direction::Input, com::ComEndpoint::BROADCAST, devbyte::BOARD }, PARAMETER_CANFD_SET_DATA_RATE, ByteOperations::toBytes(data_rate));
  std::this_thread::sleep_for(SET_PARAMETER_DELAY); // MCU has to restart CAN controller after changing bitrate, wait a bit before trying to fetch the parameter again.

  unsigned int currentDataRate = 0;
  bool success                 = getDataRate(currentDataRate);
  success &= (data_rate == 0) || (currentDataRate == data_rate);

  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set SocketCAN data rate on interface " + _id.name + " to " + std::to_string(currentDataRate));
  } else {
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Warning,
        "Failed to set SocketCAN data rate on interface " + _id.name + " to " + std::to_string(data_rate) + ". Actual value is " + std::to_string(currentDataRate) + ". May be due to quantization, check returned value manually.");
    // return false;
  }

  return true;
}

bool CanInterface::getDataRate(unsigned int& data_rate) {
  _got_update = false;

  send(com::ComEndpoint{ com::Direction::Input, FIRST_BOARD_ADDRESS, devbyte::BOARD }, PARAMETER_CANFD_GET_DATA_RATE, {});

  auto now = std::chrono::steady_clock::now();
  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(GET_PARAMETER_SLEEP);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got SocketCAN data rate update on interface " + _id.name + ": " + std::to_string(_params.data_bitrate));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get SocketCAN data rate on interface " + _id.name);
    return false;
  }

  data_rate = _params.data_bitrate;
  return true;
}

bool CanInterface::setDataSamplePoint(float data_sample_point) {
  if (data_sample_point < 0.0f || data_sample_point > 1.0f) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid SocketCAN data sample point value on interface " + _id.name + ": " + std::to_string(data_sample_point) + ". Must be between 0 and 1.");
    return false;
  }

  send(com::ComEndpoint{ com::Direction::Input, com::ComEndpoint::BROADCAST, devbyte::BOARD }, PARAMETER_CANFD_SET_SAMPLE_POINT, ByteOperations::toBytes(data_sample_point));
  std::this_thread::sleep_for(SET_PARAMETER_DELAY); // MCU has to restart CAN controller after changing sample point, wait a bit before trying to fetch the parameter again.

  float currentDataSamplePoint = 0.0f;
  bool success                 = getDataSamplePoint(currentDataSamplePoint);
  success &= (data_sample_point == 0) || (currentDataSamplePoint == data_sample_point);

  if (success) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Set SocketCAN data sample point on interface " + _id.name + " to " + std::to_string(currentDataSamplePoint));
  } else {
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Warning, "Failed to set SocketCAN data sample point on interface " + _id.name + " to " + std::to_string(data_sample_point) + ". Actual value is " + std::to_string(currentDataSamplePoint)
                                           + ". May be due to quantization, check returned value manually.");
    // return false;
  }

  return true;
}

bool CanInterface::getDataSamplePoint(float& data_sample_point) {
  _got_update = false;

  send(com::ComEndpoint{ com::Direction::Input, FIRST_BOARD_ADDRESS, devbyte::BOARD }, PARAMETER_CANFD_GET_SAMPLE_POINT, {});

  auto now = std::chrono::steady_clock::now();
  while (!_got_update && std::chrono::steady_clock::now() - now < GET_PARAMETER_TIMEOUT) {
    std::this_thread::sleep_for(GET_PARAMETER_SLEEP);
  }

  if (_got_update) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Got SocketCAN data sample point update on interface " + _id.name + ": " + std::to_string(_params.data_sample_point));
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to get SocketCAN data sample point on interface " + _id.name);
    return false;
  }

  data_sample_point = _params.data_sample_point;
  return true;
}

void CanInterface::comCallback(const com::ComEndpoint, std::uint8_t command, const std::vector<uint8_t>& data) {
  LockGuard lock(_param_mutex);

  switch (command) {
  case PARAMETER_CANFD_GET_BRS: {
    if (data.size() >= 1) {
      _params.respond_with_brs = static_cast<bool>(data[0]);
      _got_update              = true;
    }
    return;
  }

  case PARAMETER_CANFD_GET_DATA_RATE: {
    if (data.size() >= 4) {
      _params.data_bitrate = ByteOperations::readUint32(data, 0);
      _got_update          = true;
    }
    return;
  }

  case PARAMETER_CANFD_GET_SAMPLE_POINT: {
    if (data.size() >= 4) {
      _params.data_sample_point = ByteOperations::readFloat(data, 0);
      _got_update               = true;
    }
    return;
  }
  }
}

} // namespace com

} // namespace sensorring

} // namespace eduart
