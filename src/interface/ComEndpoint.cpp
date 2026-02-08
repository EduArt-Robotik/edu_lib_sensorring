#include "sensorring/interface/ComEndpoint.hpp"

namespace eduart {

namespace com {

ComEndpoint::ComEndpoint(const std::string& id)
    : _id(id) {};

ComEndpoint::ComEndpoint(const ComEndpoint& endpoint)
    : _id(endpoint._id) {};

const std::string ComEndpoint::getId() const {
  return _id;
}

bool ComEndpoint::operator==(const ComEndpoint& other) const {
  return _id == other._id;
}

} // namespace com

} // namespace eduart
