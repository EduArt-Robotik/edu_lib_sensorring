#include "sensorring/device/light/Light.hpp"

namespace eduart {

namespace sensorring {

namespace device {

Light::Light(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable)
    : Device(id, interface, target, enable) {
}

} // namespace device

} // namespace sensorring

} // namespace eduart
