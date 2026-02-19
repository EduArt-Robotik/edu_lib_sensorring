#include "ComManager.hpp"

#include "sensorring/logger/Logger.hpp"

#ifdef USE_SOCKETCAN
#include "can/SocketCANFD.hpp"
#endif

#ifdef USE_USBTINGO
#include "can/USBtingo.hpp"
#endif

#include <algorithm>

namespace eduart {

namespace com {

ComManager* ComManager::getInstance() noexcept {
  static ComManager* instance = new ComManager;
  return instance;
}

ComInterface* ComManager::getInterface(com::ComInterfaceID id, bool create_if_unknown) {

  // Check if interface already exists
  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&id](const auto& it) {
    return (it->getID() == id);
  });
  if (it != _interfaces.end())
    return it->get();

  if (!create_if_unknown) {
    return nullptr;
  }

  // No interface options specified
#if !(defined(USE_SOCKETCAN) || defined(USE_USBTINGO))
  logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Built sensorring library without any interface options. Unable to open any communication interface.");
#endif

  // Interface does not exist, create a new one
  switch (id.type) {
  case InterfaceType::SOCKETCAN:
#ifdef USE_SOCKETCAN
    _interfaces.emplace_back(std::make_unique<SocketCANFD>(id.name));
    break;
#else
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Requested to open a SocketCAN interface, but the sensorring library is built without -DUSE_SOCKETCAN=ON option.");
    return nullptr;
#endif

  case InterfaceType::USBTINGO:
#ifdef USE_USBTINGO
    _interfaces.emplace_back(std::make_unique<USBtingo>(id.name));
    break;
#else
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Requested to open a USBtingo interface, but  the sensorring library is built without -DUSE_USBTINGO=ON option.");
    return nullptr;
#endif

  case InterfaceType::UNDEFINED:
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Got an undefined interface type. Trying to open a the interface by its name.");
    try {
#ifdef USE_SOCKETCAN
      _interfaces.emplace_back(std::make_unique<SocketCANFD>(id.name));
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Successfully opened SocketCAN interface by name. Please correct the interface type in the parameters.");
      break;
#endif
    } catch (...) {
    }

    try {
#ifdef USE_USBTINGO
      _interfaces.emplace_back(std::make_unique<USBtingo>(id.name));
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Successfully opened USBtingo interface by name. Please correct the interface type in the parameters.");
      break;
#endif
    } catch (...) {
    }

    // If both attempts failed, return nullptr instead of falling through to default
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open interface by name. Both SocketCAN and USBtingo attempts failed or are not available.");
    return nullptr;

  default:
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to open unknown interface type.");
    return nullptr;
  }

  return _interfaces.back().get();
}

std::vector<ComInterface*> ComManager::getInterfaces() {
  std::vector<ComInterface*> interfaces;
  for (const auto& interface : _interfaces) {
    interfaces.push_back(interface.get());
  }
  return interfaces;
}

} // namespace com

} // namespace eduart