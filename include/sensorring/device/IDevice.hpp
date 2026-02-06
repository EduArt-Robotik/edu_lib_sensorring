#pragma once

#include <algorithm>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "CapabilityException.hpp"
#include "ICapability.hpp"
#include "ICapabilityAsync.hpp"

namespace eduart {

namespace device {

/**
 * @class IInvokerBase
 * @brief Base interface for all capability invokers used by IDevice.
 */
struct SENSORRING_EXPORT IInvokerBase {
  virtual ~IInvokerBase() = default;
  std::string name;
  const std::string& get_name() const { return name; }
};

/**
 * @brief Concrete invoker storage for capability type Cap.
 * @tparam Cap Capability type.
 */
template <typename Cap> struct SENSORRING_EXPORT Invoker : IInvokerBase {
  ICapability<Cap>* impl                        = nullptr;
  const ICapability<Cap>* impl_const            = nullptr;
  ICapabilityAsync<Cap>* impl_async             = nullptr;
  const ICapabilityAsync<Cap>* impl_async_const = nullptr;
};

/**
 * @class IDevice
 * @brief Base interface for devices exposing typed capabilities.
 */
struct SENSORRING_EXPORT IDevice {
  virtual ~IDevice() = default;

  /**
   * @brief List all registered capabilities.
   * @return Vector of (type_index, name) pairs for each capability.
   */
  std::vector<std::pair<std::type_index, std::string> > capabilities() const;

  /**
   * @brief Check if capability Cap is registered on this device.
   * @tparam Cap Capability type.
   * @return true if Cap is registered, false otherwise.
   */
  template <typename Cap> bool supports() const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return false;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());
    return inv->impl || inv->impl_const || inv->impl_async || inv->impl_async_const;
  }

  // -------------------------
  // Non-throwing synchronous invoke (non-const)
  // Returns std::optional<Response> (std::nullopt if not supported)
  // -------------------------
  /**
   * @brief Invoke capability Cap synchronously.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; empty if unsupported.
   */
  template <typename Cap> std::optional<typename Cap::Response> invoke(const typename Cap::Request& req) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return std::nullopt;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());

    if (inv->impl) {
      return inv->impl->invoke(req);
    }
    if (inv->impl_async) {
      return inv->impl_async->invoke_async(req).get();
    }
    if (inv->impl_const) {
      return inv->impl_const->invoke(req);
    }
    if (inv->impl_async_const) {
      return inv->impl_async_const->invoke_async(req).get();
    }
    return std::nullopt;
  }

  // -------------------------
  // Non-throwing synchronous invoke (const)
  // -------------------------
  /**
   * @brief Const overload of synchronous invoke.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; empty if unsupported.
   */
  template <typename Cap> std::optional<typename Cap::Response> invoke(const typename Cap::Request& req) const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return std::nullopt;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());

    if (inv->impl_const) {
      return inv->impl_const->invoke(req);
    }
    if (inv->impl_async_const) {
      return inv->impl_async_const->invoke_async(req).get();
    }
    if (inv->impl) {
      return inv->impl->invoke(req); // may mutate
    }
    if (inv->impl_async) {
      return inv->impl_async->invoke_async(req).get();
    }
    return std::nullopt;
  }

  // -------------------------
  // Non-throwing asynchronous invoke (non-const)
  // Returns std::optional<std::future<Response>>
  // -------------------------
  /**
   * @brief Invoke capability Cap asynchronously.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::optional<std::future<Response>>; empty if unsupported.
   */
  template <typename Cap> std::optional<std::future<typename Cap::Response> > invoke_async(const typename Cap::Request& req) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return std::nullopt;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());

    if (inv->impl_async) {
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async->invoke_async(req));
    }
    if (inv->impl_async_const) {
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async_const->invoke_async(req));
    }
    if (inv->impl) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [impl = inv->impl, req]() {
        return impl->invoke(req);
      }));
    }
    if (inv->impl_const) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [impl = inv->impl_const, req]() {
        return impl->invoke(req);
      }));
    }
    return std::nullopt;
  }

  // -------------------------
  // Non-throwing asynchronous invoke (const)
  // -------------------------
  /**
   * @brief Const overload of asynchronous invoke.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::optional<std::future<Response>>; empty if unsupported.
   */
  template <typename Cap> std::optional<std::future<typename Cap::Response> > invoke_async(const typename Cap::Request& req) const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return std::nullopt;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());

    if (inv->impl_async_const) {
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async_const->invoke_async(req));
    }
    if (inv->impl_async) {
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async->invoke_async(req));
    }
    if (inv->impl_const) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [impl = inv->impl_const, req]() {
        return impl->invoke(req);
      }));
    }
    if (inv->impl) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [impl = inv->impl, req]() {
        return impl->invoke(req);
      }));
    }
    return std::nullopt;
  }

  // -------------------------
  // Throwing wrappers
  // - try_invoke returns Response or throws CapabilityNotSupported
  // - try_invoke_async returns std::future<Response> or throws
  // -------------------------
  /**
   * @brief Invoke capability Cap synchronously or throw if unsupported.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw CapabilityNotSupported if Cap is not registered.
   */
  template <typename Cap> typename Cap::Response try_invoke(const typename Cap::Request& req) {
    auto opt = invoke<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Const overload of try_invoke.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw CapabilityNotSupported if Cap is not registered.
   */
  template <typename Cap> typename Cap::Response try_invoke(const typename Cap::Request& req) const {
    auto opt = invoke<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Invoke capability Cap asynchronously or throw if unsupported.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw CapabilityNotSupported if Cap is not registered.
   */
  template <typename Cap> std::future<typename Cap::Response> try_invoke_async(const typename Cap::Request& req) {
    auto opt = invoke_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Const overload of try_invoke_async.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw CapabilityNotSupported if Cap is not registered.
   */
  template <typename Cap> std::future<typename Cap::Response> try_invoke_async(const typename Cap::Request& req) const {
    auto opt = invoke_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

protected:
  /**
   * @brief Register a synchronous capability Cap implemented by this device.
   * @tparam Cap Capability type.
   * @param[in] name Optional human-readable name.
   */
  template <typename Cap> void register_capability(const std::string& name = {}) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end()) {
      auto inv        = std::make_unique<Invoker<Cap> >();
      inv->impl       = dynamic_cast<ICapability<Cap>*>(this);
      inv->impl_const = dynamic_cast<const ICapability<Cap>*>(this);
      inv->name       = name.empty() ? typeid(Cap).name() : name;
      invokers_.emplace(idx, std::move(inv));
    } else {
      auto inv        = static_cast<Invoker<Cap>*>(it->second.get());
      inv->impl       = dynamic_cast<ICapability<Cap>*>(this);
      inv->impl_const = dynamic_cast<const ICapability<Cap>*>(this);
      if (!name.empty())
        inv->name = name;
    }
  }

  /**
   * @brief Register an asynchronous capability Cap implemented by this device.
   * @tparam Cap Capability type.
   * @param[in] name Optional human-readable name.
   */
  template <typename Cap> void register_capability_async(const std::string& name = {}) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end()) {
      auto inv              = std::make_unique<Invoker<Cap> >();
      inv->impl_async       = dynamic_cast<ICapabilityAsync<Cap>*>(this);
      inv->impl_async_const = dynamic_cast<const ICapabilityAsync<Cap>*>(this);
      inv->name             = name.empty() ? typeid(Cap).name() : name;
      invokers_.emplace(idx, std::move(inv));
    } else {
      auto inv              = static_cast<Invoker<Cap>*>(it->second.get());
      inv->impl_async       = dynamic_cast<ICapabilityAsync<Cap>*>(this);
      inv->impl_async_const = dynamic_cast<const ICapabilityAsync<Cap>*>(this);
      if (!name.empty())
        inv->name = name;
    }
  }

private:
  // Map from capability type_index to invoker storage.
  std::unordered_map<std::type_index, std::unique_ptr<IInvokerBase> > invokers_;
};

} // namespace device

} // namespace eduart