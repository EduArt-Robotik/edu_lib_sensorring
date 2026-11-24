#include "ComManager.hpp"

#include "sensorring/logger/Logger.hpp"

#ifdef USE_SOCKETCAN
#include "can/socketcan/SocketCANFD.hpp"
#endif

#ifdef USE_USBTINGO
#include "can/usbtingo/USBtingo.hpp"
#endif

#include "can/ethernet/Ethernet2Can.hpp"

#include <algorithm>
#include <charconv>

namespace eduart {

namespace sensorring {

namespace com {

namespace {
bool parseEthernet2CanInterfaceName(const std::string& name, Ethernet2CanParams& params) {
  const auto host_separator = name.find(':');
  const auto port_separator = name.find('/', host_separator == std::string::npos ? 0 : host_separator + 1);
  const auto bus_separator  = name.find(':', port_separator == std::string::npos ? 0 : port_separator + 1);
  if (host_separator == std::string::npos || port_separator == std::string::npos || bus_separator == std::string::npos || host_separator == 0) {
    return false;
  }

  unsigned int port = 0;
  unsigned int bus_index = 0;
  unsigned int local_port = 0;
  const auto parse_number = [](const std::string& value, unsigned int& output) {
    const auto result = std::from_chars(value.data(), value.data() + value.size(), output);
    return result.ec == std::errc{} && result.ptr == value.data() + value.size();
  };
  if (!parse_number(name.substr(host_separator + 1, port_separator - host_separator - 1), port)
      || !parse_number(name.substr(port_separator + 1, bus_separator - port_separator - 1), bus_index)
      || !parse_number(name.substr(bus_separator + 1), local_port)
      || port == 0 || port > 65535 || bus_index > 255 || local_port > 65535) {
    return false;
  }

  params = Ethernet2CanParams(name.substr(0, host_separator), static_cast<unsigned short>(port), static_cast<unsigned char>(bus_index), static_cast<unsigned short>(local_port));
  return true;
}
} // namespace

ComManager* ComManager::getInstance() noexcept {
  // Intentional leak: the singleton is allocated once and never deleted.
  // This avoids the static destruction order fiasco, ensuring the ComManager
  // remains available until process exit.
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
#if !(defined(USE_SOCKETCAN) || defined(USE_USBTINGO) || defined(USE_ETHERNET2CAN))
  logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Built sensorring library without any interface options. Unable to open any communication interface.");
#endif

  // Interface does not exist, create a new one
  switch (id.type) {
  case InterfaceType::SocketCan:
#ifdef USE_SOCKETCAN
    _interfaces.emplace_back(std::make_unique<SocketCANFD>(id.name));
    break;
#else
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Requested to open a SocketCAN interface, but the sensorring library is built without -DSENSORRING_USE_SOCKETCAN=ON option.");
    return nullptr;
#endif

  case InterfaceType::UsbTingo:
#ifdef USE_USBTINGO
    _interfaces.emplace_back(std::make_unique<USBtingo>(id.name));
    break;
#else
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Requested to open a USBtingo interface, but  the sensorring library is built without -DSENSORRING_USE_USBTINGO=ON option.");
    return nullptr;
#endif

  case InterfaceType::Ethernet2Can:
    {
      Ethernet2CanParams params;
      if (!parseEthernet2CanInterfaceName(id.name, params)) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Invalid Ethernet CAN interface identifier.");
        return nullptr;
      }
      _interfaces.emplace_back(std::make_unique<Ethernet2Can>(params));
      break;
    }

  case InterfaceType::Undefined:
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

ComInterface* ComManager::getInterface(const com::SocketCanParams& params) {
#ifdef USE_SOCKETCAN
  com::ComInterfaceID id{ InterfaceType::SocketCan, params.name };

  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&id](const auto& it) {
    return (it->getID() == id);
  });
  if (it != _interfaces.end())
    return it->get();

  _interfaces.emplace_back(std::make_unique<SocketCANFD>(params));
  return _interfaces.back().get();
#else
  (void)params;
  logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Requested to open a SocketCAN interface, but the sensorring library is built without -DSENSORRING_USE_SOCKETCAN=ON option.");
  return nullptr;
#endif
}

ComInterface* ComManager::getInterface(const com::UsbTingoParams& params) {
#ifdef USE_USBTINGO
  com::ComInterfaceID id{ InterfaceType::UsbTingo, params.name };

  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&id](const auto& it) {
    return (it->getID() == id);
  });
  if (it != _interfaces.end())
    return it->get();

  _interfaces.emplace_back(std::make_unique<USBtingo>(params));
  return _interfaces.back().get();
#else
  (void)params;
  logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Requested to open a USBtingo interface, but the sensorring library is built without -DSENSORRING_USE_USBTINGO=ON option.");
  return nullptr;
#endif
}

ComInterface* ComManager::getInterface(const com::Ethernet2CanParams& params) {
  com::ComInterfaceID id{ InterfaceType::Ethernet2Can, params.interfaceName() };

  const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&id](const auto& interface) {
    return interface->getID() == id;
  });
  if (it != _interfaces.end())
    return it->get();

  _interfaces.emplace_back(std::make_unique<Ethernet2Can>(params));
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

} // namespace sensorring

} // namespace eduart