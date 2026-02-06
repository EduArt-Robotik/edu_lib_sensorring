#include "sensorring/device/IDevice.hpp"

namespace eduart {

namespace device {

std::vector<std::pair<std::type_index, std::string> > IDevice::capabilities() const {
  std::vector<std::pair<std::type_index, std::string> > out;
  out.reserve(invokers_.size());
  for (auto const& kv : invokers_) {
    out.emplace_back(kv.first, kv.second->get_name().empty() ? kv.first.name() : kv.second->get_name());
  }
  return out;
}

} // namespace device

} // namespace eduart