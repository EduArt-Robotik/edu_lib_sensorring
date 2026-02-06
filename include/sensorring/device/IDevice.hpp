// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   IDevice.hpp
 * @author EduArt Robotik GmbH
 * @brief  Device interface exposing typed capabilities via instance implementations, registered functions, or global static functions
 * @date   2025-02-06
 */

#pragma once

#include <algorithm>
#include <functional>
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
struct IInvokerBase {
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
template <typename Cap> struct Invoker : IInvokerBase {
  ICapability<Cap>* impl                        = nullptr;
  const ICapability<Cap>* impl_const            = nullptr;
  ICapabilityAsync<Cap>* impl_async             = nullptr;
  const ICapabilityAsync<Cap>* impl_async_const = nullptr;
  std::function<typename Cap::Response(const typename Cap::Request&)> func;
  std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> func_async;
};

/**
 * @class IDevice
 * @brief Base interface for devices exposing typed capabilities via ICapability/ICapabilityAsync, registered instance functions, or global static functions.
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
   * @note Instance invoke methods fall back to global static functions if not found on the instance.
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
   * @brief Invoke capability Cap synchronously (instance, registered function, or global static function fallback).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; std::nullopt if unsupported.
   */
  template <typename Cap> std::optional<typename Cap::Response> invoke(const typename Cap::Request& req) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it != invokers_.end()) {
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
    }
    // fallback to static (global) function if any
    if (auto sf = static_get_function<Cap>())
      return sf->get()(req);
    if (auto sf_async = static_get_function_async<Cap>())
      return sf_async->get()(req).get();
    return std::nullopt;
  }

  /**
   * @brief Const overload of synchronous invoke (instance, registered function, or global static function fallback).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; std::nullopt if unsupported.
   */
  template <typename Cap> std::optional<typename Cap::Response> invoke(const typename Cap::Request& req) const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it != invokers_.end()) {
      auto inv = static_cast<Invoker<Cap>*>(it->second.get());
      if (inv->impl_const)
        return inv->impl_const->invoke(req);
      if (inv->func)
        return inv->func(req); // registered free/static considered const-friendly
      if (inv->impl_async_const)
        return inv->impl_async_const->invoke_async(req).get();
      if (inv->func_async)
        return inv->func_async(req).get();
      if (inv->impl)
        return inv->impl->invoke(req);
      if (inv->impl_async)
        return inv->impl_async->invoke_async(req).get();
    }
    // fallback to static
    if (auto sf = static_get_function<Cap>())
      return sf->get()(req);
    if (auto sf_async = static_get_function_async<Cap>())
      return sf_async->get()(req).get();
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap asynchronously (instance, registered async function, or global static function fallback; sync run in background).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::optional<std::future<Response>>; empty if unsupported.
   */
  template <typename Cap> std::optional<std::future<typename Cap::Response> > invoke_async(const typename Cap::Request& req) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it != invokers_.end()) {
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
    }
    // static
    if (auto sf_async = static_get_function_async<Cap>())
      return std::optional<std::future<typename Cap::Response> >(sf_async->get()(req));
    if (auto sf = static_get_function<Cap>()) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [f = sf->get(), req]() {
        return f(req);
      }));
    }
    return std::nullopt;
  }

  /**
   * @brief Const overload of asynchronous invoke (instance, registered async function, or global static function fallback; sync run in background).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::optional<std::future<Response>>; empty if unsupported.
   */
  template <typename Cap> std::optional<std::future<typename Cap::Response> > invoke_async(const typename Cap::Request& req) const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it != invokers_.end()) {
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
    }
    // static
    if (auto sf_async = static_get_function_async<Cap>())
      return std::optional<std::future<typename Cap::Response> >(sf_async->get()(req));
    if (auto sf = static_get_function<Cap>()) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [f = sf->get(), req]() {
        return f(req);
      }));
    }
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap synchronously or throw if unsupported (instance, registered function, or static fallback).
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
   * @brief Const overload of try_invoke (instance, registered function, or static fallback).
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
   * @brief Invoke capability Cap asynchronously or throw if unsupported (instance, registered function, or static fallback).
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
   * @brief Const overload of try_invoke_async (instance, registered function, or static fallback).
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

  /**
   * @brief Register a global static synchronous function for capability Cap (invocable without an IDevice instance).
   * @tparam Cap Capability type.
   * @tparam F Callable type with signature compatible to Response(const Request&).
   * @param[in] f Callable to register globally for Cap.
   */
  template <typename Cap, typename F> static void register_static_function(F&& f) { static_func<Cap>() = std::function<typename Cap::Response(const typename Cap::Request&)>(std::forward<F>(f)); }

  /**
   * @brief Register a global static asynchronous function for capability Cap (invocable without an IDevice instance).
   * @tparam Cap Capability type.
   * @tparam F Callable type with signature compatible to std::future<Response>(const Request&).
   * @param[in] f Callable to register globally for Cap.
   */
  template <typename Cap, typename F> static void register_static_function_async(F&& f) { static_func_async<Cap>() = std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>(std::forward<F>(f)); }

  /**
   * @brief Invoke capability Cap synchronously via global static function (no IDevice instance required).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Response wrapped in std::optional; std::nullopt if no static function registered.
   */
  template <typename Cap> static std::optional<typename Cap::Response> invoke_static(const typename Cap::Request& req) {
    if (auto sf = static_get_function<Cap>())
      return sf->get()(req);
    if (auto sf_async = static_get_function_async<Cap>())
      return sf_async->get()(req).get();
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap asynchronously via global static function (no IDevice instance required).
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::optional<std::future<Response>>; empty if no static function registered.
   */
  template <typename Cap> static std::optional<std::future<typename Cap::Response> > invoke_static_async(const typename Cap::Request& req) {
    if (auto sf_async = static_get_function_async<Cap>())
      return std::optional<std::future<typename Cap::Response> >(sf_async->get()(req));
    if (auto sf = static_get_function<Cap>()) {
      return std::optional<std::future<typename Cap::Response> >(std::async(std::launch::async, [f = sf->get(), req]() {
        return f(req);
      }));
    }
    return std::nullopt;
  }

  /**
   * @brief Invoke capability Cap synchronously via global static function or throw if not registered.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return Capability response.
   * @throw CapabilityNotSupported if no static function registered for Cap.
   */
  template <typename Cap> static typename Cap::Response try_invoke_static(const typename Cap::Request& req) {
    auto opt = invoke_static<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  /**
   * @brief Invoke capability Cap asynchronously via global static function or throw if not registered.
   * @tparam Cap Capability type.
   * @param[in] req Request object.
   * @return std::future<Response> for the asynchronous result.
   * @throw CapabilityNotSupported if no static function registered for Cap.
   */
  template <typename Cap> static std::future<typename Cap::Response> try_invoke_static_async(const typename Cap::Request& req) {
    auto opt = invoke_static_async<Cap>(req);
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

private:
  std::unordered_map<std::type_index, std::unique_ptr<IInvokerBase> > invokers_;

  // Helper: per-Cap static function holders (function-local static avoids type-erasure).
  template <typename Cap> static std::function<typename Cap::Response(const typename Cap::Request&)>& static_func() {
    static std::function<typename Cap::Response(const typename Cap::Request&)> f;
    return f;
  }
  template <typename Cap> static std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>& static_func_async() {
    static std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> f;
    return f;
  }

  template <typename Cap> static std::optional<std::reference_wrapper<const std::function<typename Cap::Response(const typename Cap::Request&)> > > static_get_function() {
    auto& f = static_func<Cap>();
    if (f)
      return std::cref(f);
    return std::nullopt;
  }
  template <typename Cap> static std::optional<std::reference_wrapper<const std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> > > static_get_function_async() {
    auto& f = static_func_async<Cap>();
    if (f)
      return std::cref(f);
    return std::nullopt;
  }

  template <typename Cap> static bool static_has_function() { return static_cast<bool>(static_func<Cap>()); }
};

} // namespace device

} // namespace eduart