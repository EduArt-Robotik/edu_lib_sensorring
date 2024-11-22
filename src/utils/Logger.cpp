#include "Logger.hpp"

void Logger::registerObserver(MeasurementObserver* observer){
    if(observer) _observer_vec.push_back(observer);
};

void Logger::log(const LogVerbosity verbosity, const std::string msg){
	for(auto observer : _observer_vec){
		if(observer) observer->onOutputLog(verbosity, msg);
	}
};

void Logger::log(const LogVerbosity verbosity, const std::stringstream msg){
	log(verbosity, msg.str());
};