// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   ICapabilityAsync.hpp
 * @author EduArt Robotik GmbH
 * @brief  Asynchronous capability interface for IDevice (Request/Response per capability type).
 * @date   2025-02-06
 */

#pragma once

#include <future>
#include <stdexcept>
#include <utility>

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
   * @brief Invoke the capability asynchronously. Delegates to const overload if not overridden.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw std::runtime_error if invoke async is not overridden and const invoke async is not implemented.
   */
  virtual std::future<Response> invoke_async(const Request& req) {
    try {
      return std::as_const(*this).invoke_async(req);
    } catch (const std::runtime_error& e) {
      throw std::runtime_error("try to invoke capability asynchronously, but const invoke_async is not implemented");
    }
  }

  /**
   * @brief Const overload of invoke_async; override in derived classes.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw std::runtime_error if not implemented by the derived class.
   */
  virtual std::future<Response> invoke_async(const Request& req) const {
    (void)req;
    throw std::runtime_error("const async invoke not implemented for capability");
  }

  /// Destructor.
  virtual ~ICapabilityAsync() = default;
};

} // namespace device

} // namespace eduart