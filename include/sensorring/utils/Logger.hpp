#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "SingletonTemplate.hpp"
#include "MeasurementObserver.hpp"

namespace eduart{

namespace logger{

class Logger : public Singleton<Logger>{
    public:
        ~Logger() = default;

        void registerObserver(manager::MeasurementObserver* observer);

        void log(const LogVerbosity verbosity, const std::string msg);
        void log(const LogVerbosity verbosity, const std::stringstream msg);

    private:
        friend class Singleton<Logger>;
        Logger() = default;

        std::vector<manager::MeasurementObserver*> _observer_vec;

};

}

}