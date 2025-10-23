#include "ComManager.hpp"

#include "logger/Logger.hpp"

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

  // Default behaviour
  if (type == InterfaceType::UNDEFINED) {
#if defined(WIN32) || defined(USE_USBTINGO)
    type = InterfaceType::USBTINGO;
#else
    type = InterfaceType::SOCKETCAN;
#endif
  }

  // Check if interface altready exists
  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&interface_name](const auto& interface) { return interface->getInterfaceName() == interface_name; });
  if (it != _interfaces.end())
    return it->get();

  // Interface does not exist, create a new one
  switch (type) {
  case InterfaceType::SOCKETCAN:
#ifdef USE_SOCKETCAN
    _interfaces.emplace_back(std::make_unique<SocketCANFD>(interface_name));
    break;
#else
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Error, "Requested to open a SocketCan interface, but the sensorring library is built "
                                     "without -DUSE_SOCKETCAN=ON option.");
    return nullptr;
#endif

  case InterfaceType::USBTINGO:
#ifdef USE_USBTINGO
    _interfaces.emplace_back(std::make_unique<USBtingo>(interface_name));
    break;
#else
    logger::Logger::getInstance()->log(
        logger::LogVerbosity::Error, "Requested to open a USBtingo interface, but  the sensorring library is built "
                                     "without -DUSE_USBTINGO=ON option.");
    return nullptr;
#endif

  default:
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Requested to open an unknown interface type.");
    return nullptr;
  }

  return _interfaces.back().get();
}

ComInterface* ComManager::getInterface(std::string interface_name) {

  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&interface_name](const auto& interface) { return interface->getInterfaceName() == interface_name; });
  return (it != _interfaces.end()) ? it->get() : nullptr;
}

} // namespace com

} // namespace eduart