#include "ComManager.hpp"
#include "SocketCANFD.hpp"

namespace com{


std::shared_ptr<ComInterface> ComManager::getInterface(std::string interface_name){

    std::shared_ptr<ComInterface> requested_interface;

    for(auto interface : _interfaces){
        if(interface->getInterfaceName() == interface_name){
            requested_interface = interface;
        }
    }

    if(!requested_interface){
        requested_interface = std::make_shared<SocketCANFD>(interface_name); // ToDo: Incorporate different Interface types
        _interfaces.push_back(requested_interface);
    }

    return requested_interface;
};

}; // namespace edu