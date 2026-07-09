#include "sensorring/device/light/Light.hpp"

namespace eduart {

namespace sensorring {

namespace device {

Light::Light(DeviceID id, com::ComInterface* interface, com::ComEndpoint target)
    : Device(id, interface, target) {
}

} // namespace device

} // namespace sensorring

} // namespace eduart
