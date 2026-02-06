#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

// IDevice - explicit device-type-scoped static functions variant
// - Typed invoker map (per-instance capabilities & per-instance registered functions)
// - Per-(DeviceType,Cap) static function registration and invocation:
//     IDevice::register_static_function_for<ConcreteDevice, Cap>(callable);
//     IDevice::static_invoke<ConcreteDevice, Cap>(req);            // non-throwing (std::optional)
//     IDevice::try_static_invoke<ConcreteDevice, Cap>(req);        // throwing
//   Async variants: register_static_function_async_for / static_invoke_async / try_static_invoke_async
//
// Notes:
// - Instance invoke() does NOT fall back to any static functions. Static functions are only
//   invoked via the explicit static_invoke / try_static_invoke APIs (scoped by device type).
// - register_static_function_for is thread-safe (protected by an internal mutex).
// - register_function / register_function_async allow per-instance registration (unchanged).

struct CapabilityNotSupported : std::runtime_error {
  std::type_index capability_type;
  CapabilityNotSupported(std::type_index cap)
      : std::runtime_error(std::string("Capability not supported: ") + cap.name())
      , capability_type(cap) {}
};

// ---------------------------------------------
// Capability interfaces
// ---------------------------------------------
template <typename Cap> struct ICapability {
  using Request  = typename Cap::Request;
  using Response = typename Cap::Response;

  virtual Response invoke(const Request& req) = 0;
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
  virtual std::future<Response> invoke_async(const Request& req) const {
    (void)req;
    throw std::runtime_error("const async invoke not implemented for capability");
  }
  virtual ~ICapabilityAsync() = default;
};

// ---------------------------------------------
// Invoker storage (per-capability type-erased holder)
// ---------------------------------------------
struct IInvokerBase {
  virtual ~IInvokerBase() = default;
  std::string name;
  const std::string& get_name() const { return name; }
};

template <typename Cap> struct Invoker : IInvokerBase {
  // instance-based implementations
  ICapability<Cap>* impl                        = nullptr;
  const ICapability<Cap>* impl_const            = nullptr;
  ICapabilityAsync<Cap>* impl_async             = nullptr;
  const ICapabilityAsync<Cap>* impl_async_const = nullptr;

  // per-instance registered callables
  std::function<typename Cap::Response(const typename Cap::Request&)> func;
  std::function<std::future<typename Cap::Response>(const typename Cap::Request&)> func_async;
};

// ---------------------------------------------
// IDevice
// ---------------------------------------------
struct IDevice {
  virtual ~IDevice() = default;

  // Instance discovery: type_index and optional human name
  std::vector<std::pair<std::type_index, std::string> > capabilities() const {
    std::vector<std::pair<std::type_index, std::string> > out;
    out.reserve(invokers_.size());
    for (auto const& kv : invokers_) {
      out.emplace_back(kv.first, kv.second->get_name().empty() ? kv.first.name() : kv.second->get_name());
    }
    return out;
  }

  // Instance supports() (checks instance map + per-instance registered functions)
  template <typename Cap> bool supports() const {
    auto idx = std::type_index(typeid(Cap));
    auto it  = invokers_.find(idx);
    if (it == invokers_.end())
      return false;
    auto inv = static_cast<Invoker<Cap>*>(it->second.get());
    return inv->impl || inv->impl_const || inv->impl_async || inv->impl_async_const || (bool)inv->func || (bool)inv->func_async;
  }

  // -------------------------
  // Instance non-throwing invoke (sync)
  // -------------------------
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

  // -------------------------
  // Instance non-throwing invoke_async
  // -------------------------
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

  // -------------------------
  // Instance throwing wrappers (thin)
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
  // Instance registration helpers
  // -------------------------
protected:
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

  // ---------------------------------------------
  // Per-(DeviceType,Cap) static function storage & APIs
  // ---------------------------------------------
public:
  // Register a sync static/free callable for capability Cap scoped to device type D.
  // Example: IDevice::register_static_function_for<LightDevice, GetFirmware>(&LightDevice::static_get_firmware);
  template <typename D, typename Cap, typename F> static void register_static_function_for(F&& f) {
    std::lock_guard<std::mutex> lk(static_registration_mutex());
    static_per_device_sync_map<Cap>()[std::type_index(typeid(D))] = std::function<typename Cap::Response(const typename Cap::Request&)>(std::forward<F>(f));
  }

  template <typename D, typename Cap, typename F> static void register_static_function_async_for(F&& f) {
    std::lock_guard<std::mutex> lk(static_registration_mutex());
    static_per_device_async_map<Cap>()[std::type_index(typeid(D))] = std::function<std::future<typename Cap::Response>(const typename Cap::Request&)>(std::forward<F>(f));
  }

  // Non-throwing static invoke scoped to device type D
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

  // Throwing static wrappers
  template <typename D, typename Cap> static typename Cap::Response try_static_invoke(const typename Cap::Request& req) {
    auto opt = static_invoke<D, Cap>(req);
    if (opt)
      return *opt;
    throw CapabilityNotSupported(typeid(Cap));
  }

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