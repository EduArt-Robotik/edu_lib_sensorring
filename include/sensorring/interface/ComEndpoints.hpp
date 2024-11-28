#pragma once

#include <string>
#include <vector>

namespace com{

class ComEndpoint{
	public:
		ComEndpoint(const std::string& id) : _id(id) {};

		ComEndpoint(const ComEndpoint& endpoint) : _id(endpoint._id) {};
		
		const std::string getId() {return _id; };

		static std::vector<ComEndpoint> createEndpoints(std::size_t sensor_count){
			std::vector<ComEndpoint> endpoints;
			endpoints.emplace_back("tof_status");
			endpoints.emplace_back("tof_request");
			endpoints.emplace_back("thermal_status");
			endpoints.emplace_back("thermal_request");
			endpoints.emplace_back("light");
			endpoints.emplace_back("broadcast");

			for(size_t i=0; i<sensor_count; i++){
				endpoints.emplace_back("tof" + std::to_string(i) + "_data");
				endpoints.emplace_back("thermal" + std::to_string(i) + "_data");
			}

			return endpoints;
		}

		bool operator==(const ComEndpoint& other) const { return _id == other._id; }
		bool operator< (const ComEndpoint& other) const { return _id <  other._id; }

	private:
		const std::string _id;

};

}