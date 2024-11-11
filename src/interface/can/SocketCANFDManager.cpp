#include "SocketCANFDManager.hpp"

namespace edu{


std::shared_ptr<SocketCANFD> SocketCANFDManager::getInterface(std::string interface_name){

    SocketCANFD* requested_interface;

    for(auto interface : _interfaces){
        if(interface->getInterfaceName() == interface_name){
            requested_interface = interface;
        }
    }

    if(!requested_interface){
        requested_interface = std::make_shared<SocketCANDFD>(interface_name);
        _interfaces.push_back(requested_interface);
    }

    return requested_interface;
};

}; // namespace edu