#include "ComManager.hpp"

#ifdef USE_SOCKETCAN
    #include "SocketCANFD.hpp"
#endif

#ifdef USE_USBTINGO
    #include "USBtingo.hpp"
#endif

#include <algorithm>

namespace eduart{

namespace com{

ComInterface* ComManager::createInterface(DeviceType type, std::string interface_name, std::size_t sensor_count){

    // Default behaviour
    if (type == DeviceType::UNDEFINED){
#if defined(WIN32) || defined(USE_USBTINGO)
    type = DeviceType::USBTINGO;
#else
    type = DeviceType::SOCKETCAN;
#endif
    }

    // Check if interface altready exists
    const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&interface_name](const auto& interface){return interface->getInterfaceName() == interface_name;});
    if(it != _interfaces.end()) return it->get();

    // Interface does not exist, create a new one
    switch(type){
        case DeviceType::SOCKETCAN:
            #ifdef USE_SOCKETCAN
                _interfaces.emplace_back(std::make_unique<SocketCANFD>(interface_name, sensor_count));
                break;
            #else
                return nullptr;
            #endif

        case DeviceType::USBTINGO:
            #ifdef USE_USBTINGO
                _interfaces.emplace_back(std::make_unique<USBtingo>(interface_name, sensor_count));
                break;
            #else
                return nullptr;
            #endif
            
        default:
            return nullptr;
    }

    return _interfaces.back().get();
}

ComInterface* ComManager::getInterface(std::string interface_name){

    const auto& it = std::find_if(_interfaces.begin(), _interfaces.end(), [&interface_name](const auto& interface){return interface->getInterfaceName() == interface_name;});
    return (it != _interfaces.end()) ? it->get() : nullptr;
}

}

}