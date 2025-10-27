#pragma once

#include <string>
#include <set>

namespace eduart {

namespace com {

class ComEndpoint {
public:
  ComEndpoint(const std::string& id)
      : _id(id){};

  ComEndpoint(const ComEndpoint& endpoint)
      : _id(endpoint._id){};

  const std::string getId() { return _id; };

  static std::set<ComEndpoint> createStaticEndpoints() {
    std::set<ComEndpoint> endpoints;
    endpoints.emplace("tof_status");
    endpoints.emplace("tof_request");
    endpoints.emplace("thermal_status");
    endpoints.emplace("thermal_request");
    endpoints.emplace("light");
    endpoints.emplace("broadcast");

    return endpoints;
  }

  bool operator==(const ComEndpoint& other) const { return _id == other._id; }
  bool operator<(const ComEndpoint& other) const { return _id < other._id; }

private:
  const std::string _id;
};

} // namespace com

} // namespace eduart