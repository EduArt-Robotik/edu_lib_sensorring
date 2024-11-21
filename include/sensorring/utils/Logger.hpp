#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "CustomTypes.hpp"
#include "SingletonTemplate.hpp"
#include "ObserverInterface.hpp"

namespace logger{

class Logger : public Singleton<Logger>{
    public:
        ~Logger() = default;

        void registerObserver(MeasurementObserver* observer);

        void log(const LogVerbosity verbosity, const std::string msg);
        void log(const LogVerbosity verbosity, const std::stringstream msg);

    private:
        friend class Singleton<Logger>;
        Logger() = default;

        std::vector<MeasurementObserver*> _observer_vec;

}; // class Logger

}; // namespace log