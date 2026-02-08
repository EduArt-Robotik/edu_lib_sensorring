// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   IDevice.hpp
 * @author EduArt Robotik GmbH
 * @brief  Device interface exposing typed capabilities via instance implementations, registered functions, or static functions
 * @date   2025-02-06
 */

#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
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
  /**
   * @brief Get the invoker's human-readable name.
   * @return Name string for this capability invoker.
   */
  const std::string& get_name() const { return name; }
};

/**
 * @class Invoker
 * @brief Storage for one capability: either instance-based ICapability/ICapabilityAsync or user-registered callables (free/static/lambda).
 * @tparam Cap Capability type providing nested Request and Response types.
 */
template <typename Cap> struct SENSORRING_EXPORT Invoker : IInvokerBase {
  ICapability<Cap>* impl                        = nullptr;
  const ICapability<Cap>* impl_const            = nullptr;
  ICapabilityAsync<Cap>* impl_async             = nullptr;
  const ICapabilityAsync<Cap>* impl_async_const = nullptr;
  std::function<typename Cap::Response(const typename Cap::Request&)> func;
  std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> func_async;
};

/**
 * @class IDevice
 * @brief Base interface for devices exposing typed capabilities via ICapability/ICapabilityAsync, registered instance functions, or static functions.
 */
struct SENSORRING_EXPORT IDevice {
  virtual ~IDevice() = default;

  /**
   * @brief List all registered capabilities.
   * @return Vector of (type_index, name) pairs for each capability.
   */
  std::vector<std::pair<std::type_index, std::string> > capabilities() const;

  /**
   * @brief Check if capability Cap is provided by this instance (instance implementation or registered function).
   * @tparam Cap Capability type.
   * @return true if Cap is provided by any ICapability, ICapabilityAsync, or registered sync/async function on this instance; false otherwise.
   */
  template <typename Cap> bool supports() const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return false;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());
    return inv->impl || inv->impl_const || inv->impl_async || inv->impl_async_const || (bool)inv->func || (bool)inv->func_async;
  }

  /**
   * @brief Invoke capability Cap synchronously (instance or registered function on this device).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; std::nullopt if unsupported.
   */
  template <typename Cap> std::optional<typename Cap::Response> invoke(const typename Cap::Request& req) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return std::nullopt;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());

    if (inv->impl)
      return inv->impl->invoke(req);
    if (inv->func)
      return inv->func(req);
    if (inv->impl_async)
      return inv->impl_async->invoke_async(req).get();
    if (inv->func_async)
      return inv->func_async(req).get();
    if (inv->impl_const)
      return inv->impl_const->invoke(req);
    if (inv->impl_async_const)
      return inv->impl_async_const->invoke_async(req).get();
    return std::nullopt;
  }

  /**
   * @brief Const overload of synchronous invoke (instance or registered function on this device).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; std::nullopt if unsupported.
   */
  template <typename Cap> std::optional<typename Cap::Response> invoke(const typename Cap::Request& req) const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return std::nullopt;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());

    if (inv->impl_const)
      return inv->impl_const->invoke(req);
    if (inv->func)
      return inv->func(req); // treat registered func as const-friendly
    if (inv->impl_async_const)
      return inv->impl_async_const->invoke_async(req).get();
    if (inv->func_async)
      return inv->func_async(req).get();
    if (inv->impl)
      return inv->impl->invoke(req);
    if (inv->impl_async)
      return inv->impl_async->invoke_async(req).get();
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap asynchronously (instance or registered function on this device; sync implementations run in background).
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

    if (inv->impl_async)
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async->invoke_async(req));
    if (inv->func_async)
      return std::optional<std::future<typename Cap::Response> >(inv->func_async(req));
    if (inv->impl) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [impl = inv->impl, req]() {
        return impl->invoke(req);
      }));
    }
    if (inv->func) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [f = inv->func, req]() {
        return f(req);
      }));
    }
    if (inv->impl_async_const)
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async_const->invoke_async(req));
    if (inv->impl_const) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [impl = inv->impl_const, req]() {
        return impl->invoke(req);
      }));
    }
    return std::nullopt;
  }

  /**
   * @brief Const overload of asynchronous invoke (instance or registered function on this device; sync run in background).
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

    if (inv->impl_async_const)
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async_const->invoke_async(req));
    if (inv->func_async)
      return std::optional<std::future<typename Cap::Response> >(inv->func_async(req));
    if (inv->impl_async)
      return std::optional<std::future<typename Cap::Response> >(inv->impl_async->invoke_async(req));
    if (inv->func) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [f = inv->func, req]() {
        return f(req);
      }));
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

  /**
   * @brief Invoke capability Cap synchronously or throw if unsupported (instance or registered function on this device).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw CapabilityNotSupported if Cap is not provided.
   */
  template <typename Cap> typename Cap::Response try_invoke(const typename Cap::Request& req) {
    auto opt = invoke<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Const overload of try_invoke (instance or registered function on this device).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw CapabilityNotSupported if Cap is not provided.
   */
  template <typename Cap> typename Cap::Response try_invoke(const typename Cap::Request& req) const {
    auto opt = invoke<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Invoke capability Cap asynchronously or throw if unsupported (instance or registered function on this device).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw CapabilityNotSupported if Cap is not provided.
   */
  template <typename Cap> std::future<typename Cap::Response> try_invoke_async(const typename Cap::Request& req) {
    auto opt = invoke_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Const overload of try_invoke_async (instance or registered function on this device).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw CapabilityNotSupported if Cap is not provided.
   */
  template <typename Cap> std::future<typename Cap::Response> try_invoke_async(const typename Cap::Request& req) const {
    auto opt = invoke_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

protected:
  /**
   * @brief Register a synchronous capability Cap implemented by this device (ICapability).
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
   * @brief Register an asynchronous capability Cap implemented by this device (ICapabilityAsync).
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

  /**
   * @brief Register a synchronous capability Cap implemented by a callable (free function, static function, or lambda) on this instance.
   * @tparam Cap Capability type.
   * @tparam F Callable type with signature compatible to Response(const Request&).
   * @param[in] f Callable to invoke for Cap on this instance.
   * @param[in] name Optional human-readable name.
   */
  template <typename Cap, typename F> void register_function(F&& f, const std::string& name = {}) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end()) {
      auto inv  = std::make_unique<Invoker<Cap> >();
      inv->func = std::function<typename Cap::Response(const typename Cap::Request&)>(std::forward<F>(f));
      inv->name = name.empty() ? typeid(Cap).name() : name;
      invokers_.emplace(idx, std::move(inv));
    } else {
      auto inv  = static_cast<Invoker<Cap>*>(it->second.get());
      inv->func = std::function<typename Cap::Response(const typename Cap::Request&)>(std::forward<F>(f));
      if (!name.empty())
        inv->name = name;
    }
  }

  /**
   * @brief Register an asynchronous capability Cap implemented by a callable returning std::future<Response> on this instance.
   * @tparam Cap Capability type.
   * @tparam F Callable type with signature compatible to std::future<Response>(const Request&).
   * @param[in] f Callable to invoke for Cap on this instance.
   * @param[in] name Optional human-readable name.
   */
  template <typename Cap, typename F> void register_function_async(F&& f, const std::string& name = {}) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end()) {
      auto inv        = std::make_unique<Invoker<Cap> >();
      inv->func_async = std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>(std::forward<F>(f));
      inv->name       = name.empty() ? typeid(Cap).name() : name;
      invokers_.emplace(idx, std::move(inv));
    } else {
      auto inv        = static_cast<Invoker<Cap>*>(it->second.get());
      inv->func_async = std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>(std::forward<F>(f));
      if (!name.empty())
        inv->name = name;
    }
  }

public:
  /**
   * @brief Register a synchronous static/free callable for capability Cap scoped to device type D (invocable without an instance).
   * @tparam D Device type that this static function is associated with.
   * @tparam Cap Capability type.
   * @tparam F Callable type with signature compatible to Response(const Request&).
   * @param[in] f Callable to register for (D, Cap).
   */
  template <typename D, typename Cap, typename F> static void register_static_function_for(F&& f) {
    std::lock_guard<std::mutex> lk(static_registration_mutex());
    static_per_device_sync_map<Cap>()[std::type_index(typeid(D))] = std::function<typename Cap::Response(const typename Cap::Request&)>(std::forward<F>(f));
  }

  /**
   * @brief Register an asynchronous static/free callable for capability Cap scoped to device type D (invocable without an instance).
   * @tparam D Device type that this static function is associated with.
   * @tparam Cap Capability type.
   * @tparam F Callable type with signature compatible to std::future<Response>(const Request&).
   * @param[in] f Callable to register for (D, Cap).
   */
  template <typename D, typename Cap, typename F> static void register_static_function_async_for(F&& f) {
    std::lock_guard<std::mutex> lk(static_registration_mutex());
    static_per_device_async_map<Cap>()[std::type_index(typeid(D))] = std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>(std::forward<F>(f));
  }

  /**
   * @brief Invoke capability Cap synchronously via static function registered for device type D (no instance required).
   * @tparam D Device type.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; std::nullopt if no static function registered for (D, Cap).
   */
  template <typename D, typename Cap> static std::optional<typename Cap::Response> static_invoke(const typename Cap::Request& req) {
    // sync function registered?
    auto& m = static_per_device_sync_map<Cap>();
    auto it = m.find(std::type_index(typeid(D)));
    if (it != m.end())
      return it->second(req);
    // async static function registered?
    auto& ma = static_per_device_async_map<Cap>();
    auto ita = ma.find(std::type_index(typeid(D)));
    if (ita != ma.end())
      return ita->second(req).get();
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap asynchronously via static function registered for device type D (no instance required).
   * @tparam D Device type.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::optional<std::future<Response>>; empty if no static function registered for (D, Cap).
   */
  template <typename D, typename Cap> static std::optional<std::future<typename Cap::Response> > static_invoke_async(const typename Cap::Request& req) {
    auto& ma = static_per_device_async_map<Cap>();
    auto ita = ma.find(std::type_index(typeid(D)));
    if (ita != ma.end())
      return std::optional<std::future<typename Cap::Response> >(ita->second(req));
    auto& m = static_per_device_sync_map<Cap>();
    auto it = m.find(std::type_index(typeid(D)));
    if (it != m.end()) {
      auto f = it->second;
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [f, req]() {
        return f(req);
      }));
    }
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap synchronously via static function for device type D or throw if not registered.
   * @tparam D Device type.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw CapabilityNotSupported if no static function registered for (D, Cap).
   */
  template <typename D, typename Cap> static typename Cap::Response try_static_invoke(const typename Cap::Request& req) {
    auto opt = static_invoke<D, Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Invoke capability Cap asynchronously via static function for device type D or throw if not registered.
   * @tparam D Device type.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw CapabilityNotSupported if no static function registered for (D, Cap).
   */
  template <typename D, typename Cap> static std::future<typename Cap::Response> try_static_invoke_async(const typename Cap::Request& req) {
    auto opt = static_invoke_async<D, Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<IInvokerBase> > invokers_;

  // --- static per-(DeviceType,Cap) maps + mutex ---
  static std::mutex& static_registration_mutex() {
    static std::mutex m;
    return m;
  }

  template <typename Cap> using PerDeviceSyncMap = std::unordered_map<std::type_index, std::function<typename Cap::Response(const typename Cap::Request&)> >;

  template <typename Cap> using PerDeviceAsyncMap = std::unordered_map<std::type_index, std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> >;

  template <typename Cap> static PerDeviceSyncMap<Cap>& static_per_device_sync_map() {
    static PerDeviceSyncMap<Cap> m;
    return m;
  }
  template <typename Cap> static PerDeviceAsyncMap<Cap>& static_per_device_async_map() {
    static PerDeviceAsyncMap<Cap> m;
    return m;
  }
};

} // namespace device

} // namespace eduart