#pragma once

#include <string>
#include <unordered_set>

namespace eduart {

namespace com {

class ComEndpoint {
public:
  ComEndpoint(const std::string& id)
      : _id(id) {};

  ComEndpoint(const ComEndpoint& endpoint)
      : _id(endpoint._id) {};

  const std::string getId() const { return _id; }

  bool operator==(const ComEndpoint& other) const { return _id == other._id; }

private:
  const std::string _id;
};

} // namespace com

} // namespace eduart

namespace std {

template <> struct hash<eduart::com::ComEndpoint> {
  std::size_t operator()(const eduart::com::ComEndpoint& ep) const { return std::hash<std::string>{}(ep.getId()); }
};

} // namespace std