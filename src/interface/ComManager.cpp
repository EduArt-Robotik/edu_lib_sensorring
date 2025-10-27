#include "ComManager.hpp"

#include "logger/Logger.hpp"

#include "CustomTypes.hpp"

#ifdef USE_SOCKETCAN
#include "can/SocketCANFD.hpp"
#endif

#ifdef USE_USBTINGO
#include "can/USBtingo.hpp"
#endif

#include <algorithm>

namespace eduart {

namespace com {

ComInterface* ComManager::createInterface(std::string interface_name, InterfaceType type) {

  // Check if interface altready exists
  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&interface_name](const auto& interface) {
    return interface->getInterfaceName() == interface_name;
  });
  if (it != _interfaces.end())
    return it->get();

  // No interface options specified
#if !(defined(USE_SOCKETCAN) || defined(USE_USBTINGO))
  logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Built sensorring library without any interface options. Unable to open any communication interface.");
#endif

  // Interface does not exist, create a new one
  switch (type) {
  case InterfaceType::SOCKETCAN:
#ifdef USE_SOCKETCAN
    _interfaces.emplace_back(std::make_unique<SocketCANFD>(interface_name));
    break;
#else
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Requested to open a SocketCAN interface, but the sensorring library is built without -DUSE_SOCKETCAN=ON option.");
    return nullptr;
#endif

  case InterfaceType::USBTINGO:
#ifdef USE_USBTINGO
    _interfaces.emplace_back(std::make_unique<USBtingo>(interface_name));
    break;
#else
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Requested to open a USBtingo interface, but  the sensorring library is built without -DUSE_USBTINGO=ON option.");
    return nullptr;
#endif

  case InterfaceType::UNDEFINED:
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Got an undefined interface type. Trying to open a the interface by its name.");
    try {
#ifdef USE_SOCKETCAN
      _interfaces.emplace_back(std::make_unique<SocketCANFD>(interface_name));
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Successfully opened SocketCAN interface by name. Please correct the interface type in the parameters.");
      break;
#endif
    } catch (...) {
    }

    try {
#ifdef USE_USBTINGO
      _interfaces.emplace_back(std::make_unique<USBtingo>(interface_name));
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Successfully opened USBtingo interface by name. Please correct the interface type in the parameters.");
      break;
#endif
    } catch (...) {
    }

  default:
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open unknown interface type.");
    return nullptr;
  }

  return _interfaces.back().get();
}

ComInterface* ComManager::getInterface(std::string interface_name) {

  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&interface_name](const auto& interface) {
    return interface->getInterfaceName() == interface_name;
  });
  return (it != _interfaces.end()) ? it->get() : nullptr;
}

} // namespace com

} // namespace eduart