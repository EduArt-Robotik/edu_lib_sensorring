#include "device/DeviceImpl.hpp"

namespace eduart {

namespace device {

DeviceImpl::DeviceImpl(DeviceID id)
    : _id(id)
    , _enable(false) {
}

DeviceImpl::~DeviceImpl() {
}

void DeviceImpl::comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  // ToDo: Find a way to forward the message to the derived classes
}
} // namespace device

} // namespace eduart