#pragma once

#include <stdexcept>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @class ICapability
 * @brief Synchronous capability interface used by IDevice.
 * @tparam Cap Capability type providing nested Request and Response types.
 */
template <typename Cap> struct SENSORRING_EXPORT ICapability {
  using Request  = typename Cap::Request;
  using Response = typename Cap::Response;

  /**
   * @brief Invoke the capability synchronously.
   * @param[in] req Request object.
   * @return Capability response.
   */
  virtual Response invoke(const Request& req) = 0;

  /**
   * @brief Optional const overload of invoke.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw std::runtime_error if not implemented by the derived class.
   */
  virtual Response invoke(const Request& req) const {
    (void)req;
    throw std::runtime_error("const invoke not implemented for capability");
  }

  virtual ~ICapability() = default;
};

} // namespace device

} // namespace eduart