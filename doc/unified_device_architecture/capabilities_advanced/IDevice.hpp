#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

// Exception thrown when a requested capability is not supported by a device.
struct CapabilityNotSupported : std::runtime_error {
  std::type_index capability_type;
  CapabilityNotSupported(std::type_index cap)
      : std::runtime_error(std::string("Capability not supported: ") + cap.name())
      , capability_type(cap) {}
};

// -------------------------------------------------------------
// Capability interface types (synchronous and optional async)
// -------------------------------------------------------------
template <typename Cap> struct ICapability {
  using Request  = typename Cap::Request;
  using Response = typename Cap::Response;

  virtual Response invoke(const Request& req) = 0;

  // optional const variant
  virtual Response invoke(const Request& req) const {
    (void)req;
    throw std::runtime_error("const invoke not implemented for capability");
  }

  virtual ~ICapability() = default;
};

template <typename Cap> struct ICapabilityAsync {
  using Request  = typename Cap::Request;
  using Response = typename Cap::Response;

  virtual std::future<Response> invoke_async(const Request& req) = 0;

  // optional const variant
  virtual std::future<Response> invoke_async(const Request& req) const {
    (void)req;
    throw std::runtime_error("const async invoke not implemented for capability");
  }

  virtual ~ICapabilityAsync() = default;
};

// -------------------------------------------------------------
// Type-erased invoker storage (no void*)
// Adds support for callable targets (free/static/lambda) via std::function
// -------------------------------------------------------------
struct IInvokerBase {
  virtual ~IInvokerBase() = default;
  std::string name;
  const std::string& get_name() const { return name; }
};

template <typename Cap> struct Invoker : IInvokerBase {
  // instance-based interfaces
  ICapability<Cap>* impl                        = nullptr;
  const ICapability<Cap>* impl_const            = nullptr;
  ICapabilityAsync<Cap>* impl_async             = nullptr;
  const ICapabilityAsync<Cap>* impl_async_const = nullptr;

  // callable / static / free function fallbacks (user-registered)
  std::function<typename Cap::Response(const typename Cap::Request&)> func;                    // sync callable
  std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> func_async; // async callable
};

// -------------------------------------------------------------
// IDevice (typed-invoker map)
// - invoke / invoke_async: non-throwing, return std::optional
// - try_invoke / try_invoke_async: thin wrappers that throw on missing capability
// - static function registration + static invocation support
// -------------------------------------------------------------
struct IDevice {
  virtual ~IDevice() = default;

  // Runtime discovery: list available capabilities (type_index + name)
  std::vector<std::pair<std::type_index, std::string> > capabilities() const {
    std::vector<std::pair<std::type_index, std::string> > out;
    out.reserve(invokers_.size());
    for (auto const& kv : invokers_) {
      const auto& idx      = kv.first;
      const auto& inv_base = kv.second;
      out.emplace_back(idx, inv_base->get_name().empty() ? idx.name() : inv_base->get_name());
    }
    return out;
  }

  // Does the device support Cap?
  template <typename Cap> bool supports() const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return false;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());
    return inv->impl || inv->impl_const || inv->impl_async || inv->impl_async_const || (bool)inv->func || (bool)inv->func_async || static_has_function<Cap>();
  }

  // -------------------------
  // Non-throwing synchronous invoke (non-const)
  // Returns std::optional<Response> (std::nullopt if not supported)
  // Instance funcs have priority; then registered instance/free functions;
  // finally falls back to static registered functions when calling via IDevice::invoke_static.
  // -------------------------
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

  // -------------------------
  // Non-throwing synchronous invoke (const)
  // -------------------------
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

  // -------------------------
  // Non-throwing asynchronous invoke (non-const)
  // Returns std::optional<std::future<Response>>
  // -------------------------
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

  // -------------------------
  // Non-throwing asynchronous invoke (const)
  // -------------------------
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

  // -------------------------
  // Throwing wrappers (thin): try_invoke
  // -------------------------
  template <typename Cap> typename Cap::Response try_invoke(const typename Cap::Request& req) {
    auto opt = invoke<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  template <typename Cap> typename Cap::Response try_invoke(const typename Cap::Request& req) const {
    auto opt = invoke<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  template <typename Cap> std::future<typename Cap::Response> try_invoke_async(const typename Cap::Request& req) {
    auto opt = invoke_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

  template <typename Cap> std::future<typename Cap::Response> try_invoke_async(const typename Cap::Request& req) const {
    auto opt = invoke_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

  // -------------------------
  // Static function registration & invocation (per-Cap)
  // These provide invocation without any IDevice instance.
  // -------------------------
  template <typename Cap, typename F> static void register_static_function(F&& f) { static_func<Cap>() = std::function<typename Cap::Response(const typename Cap::Request&)>(std::forward<F>(f)); }

  template <typename Cap, typename F> static void register_static_function_async(F&& f) { static_func_async<Cap>() = std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>(std::forward<F>(f)); }

  template <typename Cap> static std::optional<typename Cap::Response> invoke_static(const typename Cap::Request& req) {
    if (auto sf = static_get_function<Cap>())
      return sf->get()(req);
    if (auto sf_async = static_get_function_async<Cap>())
      return sf_async->get()(req).get();
    return std::nullopt;
  }

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

  template <typename Cap> static typename Cap::Response try_invoke_static(const typename Cap::Request& req) {
    auto opt = invoke_static<Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

  template <typename Cap> static std::future<typename Cap::Response> try_invoke_static_async(const typename Cap::Request& req) {
    auto opt = invoke_static_async<Cap>(req);
    if (opt)
      return std::move(*opt);
    throw CapabilityNotSupported(typeid(Cap));
  }

protected:
  // Register instance-based capabilities (as before)
  template <typename Cap> void register_capability(const std::string& name = {}) {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end()) {
      auto inv = std::make_unique<Invoker<Cap> >();
      // dynamic_cast since this-> is statically IDevice*
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

  // Register a free/static/lambda callable for Cap (sync)
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

  // Register free/static/lambda callable for Cap (async)
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