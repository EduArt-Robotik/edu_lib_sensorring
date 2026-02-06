#include "sensorring/device/IDevice.hpp"

namespace eduart {

namespace device {

std::vector<std::pair<std::type_index, std::string> > IDevice::capabilities() const {
  std::vector<std::pair<std::type_index, std::string> > out;
  out.reserve(invokers_.size());
  for (auto const& kv : invokers_) {
    const auto& idx      = kv.first;
    const auto& inv_base = kv.second;
    out.emplace_back(idx, inv_base->get_name().empty() ? idx.name() : inv_base->get_name());
  }
  return out;
}

} // namespace device

} // namespace eduart