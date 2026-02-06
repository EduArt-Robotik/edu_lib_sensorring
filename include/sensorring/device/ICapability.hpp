// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   ICapability.hpp
 * @author EduArt Robotik GmbH
 * @brief  Synchronous capability interface for IDevice (Request/Response per capability type).
 * @date   2025-02-06
 */

#pragma once

#include <stdexcept>
#include <utility>

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
   * @brief Invoke the capability synchronously. Delegates to const overload if not overridden.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw std::runtime_error if invoke is not overridden and const invoke is not implemented.
   */
  virtual Response invoke(const Request& req) {
    try {
      return std::as_const(*this).invoke(req);
    } catch (const std::runtime_error& e) {
      throw std::runtime_error("invoke not implemented for capability, fallback to const invoke is also not implemented");
    }
  }

  /**
   * @brief Const overload of invoke; override in derived classes.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw std::runtime_error if not implemented by the derived class.
   */
  virtual Response invoke(const Request& req) const {
    (void)req;
    throw std::runtime_error("const invoke not implemented for capability");
  }

  /// Destructor.
  virtual ~ICapability() = default;
};

} // namespace device

} // namespace eduart