#pragma once

#include <future>
#include <stdexcept>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @class ICapabilityAsync
 * @brief Asynchronous capability interface used by IDevice.
 * @tparam Cap Capability type providing nested Request and Response types.
 */
template <typename Cap> struct SENSORRING_EXPORT ICapabilityAsync {
  using Request  = typename Cap::Request;
  using Response = typename Cap::Response;

  /**
   * @brief Invoke the capability asynchronously.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   */
  virtual std::future<Response> invoke_async(const Request& req) = 0;

  /**
   * @brief Optional const overload of invoke_async.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw std::runtime_error if not implemented by the derived class.
   */
  virtual std::future<Response> invoke_async(const Request& req) const {
    (void)req;
    throw std::runtime_error("const async invoke not implemented for capability");
  }

  virtual ~ICapabilityAsync() = default;
};

} // namespace device

} // namespace eduart