#include "ComManager.hpp"
#include "SocketCANFD.hpp"

namespace com{

ComInterface* ComManager::createInterface(std::string interface_name, std::size_t sensor_count){

    ComInterface* requested_interface = nullptr;

    for(const auto& interface : _interfaces){
        if(interface->getInterfaceName() == interface_name){
            requested_interface = interface.get();
        }
    }

    if(!requested_interface){
        _interfaces.emplace_back(std::make_unique<SocketCANFD>(interface_name, sensor_count));
        requested_interface = _interfaces.back().get();
    }

    return requested_interface;
}

ComInterface* ComManager::getInterface(std::string interface_name){

    ComInterface* requested_interface = nullptr;

    for(const auto& interface : _interfaces){
        if(interface->getInterfaceName() == interface_name){
            requested_interface = interface.get();
        }
    }

    return requested_interface;
}

}; // namespace edu