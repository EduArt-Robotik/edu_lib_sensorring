#pragma once

#include <algorithm>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "CapabilityException.hpp"

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
// -------------------------------------------------------------
struct IInvokerBase {
  virtual ~IInvokerBase() = default;
  std::string name;
  const std::string& get_name() const { return name; }
};

template <typename Cap> struct Invoker : IInvokerBase {
  ICapability<Cap>* impl                        = nullptr;
  const ICapability<Cap>* impl_const            = nullptr;
  ICapabilityAsync<Cap>* impl_async             = nullptr;
  const ICapabilityAsync<Cap>* impl_async_const = nullptr;
};

// -------------------------------------------------------------
// IDevice (typed-invoker map)
// - invoke / invoke_async: non-throwing, return std::optional
// - try_invoke / try_invoke_async: thin wrappers that throw on missing capability
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
    return inv->impl || inv->impl_const || inv->impl_async || inv->impl_async_const;
  }

  // -------------------------
  // Non-throwing synchronous invoke (non-const)
  // Returns std::optional<Response> (std::nullopt if not supported)
  // -------------------------
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
  // Throwing wrappers: try_invoke
  // - try_invoke returns Response or throws CapabilityNotSupported
  // - try_invoke_async returns std::future<Response> or throws
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

protected:
  // Register capability and optional human-friendly name.
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

private:
  std::unordered_map<std::type_index, std::unique_ptr<IInvokerBase> > invokers_;
};