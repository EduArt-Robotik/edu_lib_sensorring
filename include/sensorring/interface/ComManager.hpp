#pragma once

#include <vector>
#include <string>
#include <memory>
#include "ComEndpoints.hpp"
#include "ComInterface.hpp"
#include "SingletonTemplate.hpp"

namespace com{

class ComManager : public Singleton<ComManager>{
    public:
        ComManager(const ComManager&) = delete;
        ComManager& operator=(const ComManager&) = delete;
        
        ComInterface* createInterface(std::string interface_name, std::size_t sensor_count);
        ComInterface* getInterface(std::string interface_name);

    private:
        friend class Singleton<ComManager>;
        ComManager() = default;

        std::vector<std::unique_ptr<ComInterface>> _interfaces;
};

}