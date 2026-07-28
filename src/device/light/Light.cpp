#include "sensorring/device/light/Light.hpp"

namespace eduart {

namespace sensorring {

namespace device {

Light::Light(DeviceID id, com::ComInterface* interface, com::ComEndpoint target)
    : Device(id, interface, target) {
}

void Light::setAllLights(const std::vector<Light*>& devices, LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
  for (auto* dev : devices) {
    dev->setLight(mode, red, green, blue);
  }
}

} // namespace device

} // namespace sensorring

} // namespace eduart
