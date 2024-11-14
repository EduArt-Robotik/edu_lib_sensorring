#pragma once

#include <vector>
#include <string>
#include <memory>
#include "ComInterface.hpp"
#include "SingletonTemplate.hpp"

namespace com{

class ComManager : public Singleton<ComManager>{
    public:
        ComManager(const ComManager&) = delete;
        ComManager& operator=(const ComManager&) = delete;
        
        std::shared_ptr<ComInterface> getInterface(std::string interface_name);

    private:
        friend class Singleton<ComManager>;
        ComManager() = default;

        std::vector<std::shared_ptr<ComInterface>> _interfaces;
};

}; // namespace