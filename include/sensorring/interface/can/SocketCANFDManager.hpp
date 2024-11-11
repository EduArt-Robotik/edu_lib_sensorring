#pragma once

#include <vector>
#include <string>
#include <memory>
#include "SocketCANFD.hpp"
#include "SingletonTemplate.hpp"

namespace com{

class SocketCANFDManager : Singleton<SocketCANFDManager>{
    public:
        SocketCANFDManager(const SocketCANFDManager&) = delete;
        SocketCANFDManager& operator=(const SocketCANFDManager&) = delete;
        
        std::shared_ptr<SocketCANFD> getInterface(std::string interface_name);

    private:
        friend class Singleton<SocketCANFDManager>;
        SocketCANFDManager() = default;

        std::vector<std::shared_ptr<SocketCANFD>> _instances;
};

}; // namespace