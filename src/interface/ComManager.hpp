#pragma once

#include "CustomTypes.hpp"
#include "ComEndpoints.hpp"
#include "ComInterface.hpp"
#include "utils/SingletonTemplate.hpp"

#include <vector>
#include <string>
#include <memory>

namespace eduart{

namespace com{

class ComManager : public Singleton<ComManager>{
    public:
        ComManager(const ComManager&) = delete;
        ComManager& operator=(const ComManager&) = delete;
        
        ComInterface* createInterface(std::string interface_name, InterfaceType type);
        ComInterface* getInterface(std::string interface_name);

    private:
        friend class Singleton<ComManager>;
        ComManager() = default;

        std::vector<std::unique_ptr<ComInterface>> _interfaces;
};

}

}