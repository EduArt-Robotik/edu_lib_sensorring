// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   IDeviceMacros.hpp
 * @author EduArt Robotik GmbH
 * @brief  Optional convenience macros for implementing IDevice (static and instance capability registration).
 * @date   2025-02-06
 *
 * Include this header after IDevice.hpp when you want to use the macros.
 * Static registration runs at load time (before main); use SENSORRING_REGISTER_STATIC / SENSORRING_REGISTER_STATIC_ASYNC
 * in the device's namespace (e.g. in the header or .cpp). Instance registration is for use in the device constructor.
 *
 * Cross-check vs IDevice registration API:
 * - register_capability<Cap>(name)              -> SENSORRING_REGISTER_CAPABILITY(Cap), SENSORRING_REGISTER_CAPABILITY_NAMED(Cap, Name)
 * - register_capability_async<Cap>(name)        -> SENSORRING_REGISTER_CAPABILITY_ASYNC(Cap), SENSORRING_REGISTER_CAPABILITY_ASYNC_NAMED(Cap, Name)
 * - register_function<Cap>(f, name)             -> SENSORRING_REGISTER_FUNCTION(Cap, Func), SENSORRING_REGISTER_FUNCTION_NAMED(Cap, Func, Name)
 * - register_function_async<Cap>(f, name)      -> SENSORRING_REGISTER_FUNCTION_ASYNC(Cap, Func), SENSORRING_REGISTER_FUNCTION_ASYNC_NAMED(Cap, Func, Name)
 * - register_static_function_for<D,Cap>(f)      -> SENSORRING_REGISTER_STATIC(Device, Cap, Func)
 * - register_static_function_async_for<D,Cap>(f)-> SENSORRING_REGISTER_STATIC_ASYNC(Device, Cap, Func)
 */

#pragma once

#include "IDevice.hpp"

namespace eduart {
namespace device {

// --- Internal: unique static variable name per line ---
#define SENSORRING_CAT_(a, b) a##b
#define SENSORRING_CAT(a, b)  SENSORRING_CAT_(a, b)
#define SENSORRING_STATIC_REG_VAR SENSORRING_CAT(sensorring_static_reg_, __LINE__)

/**
 * @def SENSORRING_REGISTER_STATIC(Device, Cap, Func)
 * @brief Register a synchronous static/free function for capability Cap for device type Device at load time.
 * @param Device Device type (e.g. LedLight).
 * @param Cap Capability type (e.g. SetLight).
 * @param Func Pointer to static member or free function with signature Cap::Response(const Cap::Request&).
 *
 * Use once per (Device, Cap) in the device's translation unit or header. Runs before main().
 * Example:
 * @code
 *   SENSORRING_REGISTER_STATIC(LedLight, SetLight, &LedLight::setLight);
 *   SENSORRING_REGISTER_STATIC(LedLight, SyncLight, &LedLight::syncLight);
 * @endcode
 */
#define SENSORRING_REGISTER_STATIC(Device, Cap, Func)                                      \
  static const int SENSORRING_STATIC_REG_VAR = []() {                                      \
    eduart::device::IDevice::register_static_function_for<Device, Cap>(Func);               \
    return 0;                                                                               \
  }()

/**
 * @def SENSORRING_REGISTER_STATIC_ASYNC(Device, Cap, Func)
 * @brief Register an asynchronous static/free function for capability Cap for device type Device at load time.
 * @param Device Device type.
 * @param Cap Capability type.
 * @param Func Callable with signature std::future<Cap::Response>(const Cap::Request&).
 */
#define SENSORRING_REGISTER_STATIC_ASYNC(Device, Cap, Func)                                 \
  static const int SENSORRING_STATIC_REG_VAR = []() {                                      \
    eduart::device::IDevice::register_static_function_async_for<Device, Cap>(Func);         \
    return 0;                                                                               \
  }()

/**
 * @def SENSORRING_REGISTER_FUNCTION(Cap, Func)
 * @brief Register a synchronous capability Cap implemented by a callable on this device instance (use in constructor).
 * @param Cap Capability type.
 * @param Func Member pointer or callable; signature Cap::Response(const Cap::Request&).
 */
#define SENSORRING_REGISTER_FUNCTION(Cap, Func) \
  register_function<Cap>(Func, #Cap)

/**
 * @def SENSORRING_REGISTER_FUNCTION_NAMED(Cap, Func, Name)
 * @brief Like SENSORRING_REGISTER_FUNCTION with an explicit human-readable name.
 */
#define SENSORRING_REGISTER_FUNCTION_NAMED(Cap, Func, Name) \
  register_function<Cap>(Func, Name)

/**
 * @def SENSORRING_REGISTER_FUNCTION_ASYNC(Cap, Func)
 * @brief Register an asynchronous capability Cap via callable on this instance (use in constructor).
 */
#define SENSORRING_REGISTER_FUNCTION_ASYNC(Cap, Func) \
  register_function_async<Cap>(Func, #Cap)

/**
 * @def SENSORRING_REGISTER_FUNCTION_ASYNC_NAMED(Cap, Func, Name)
 * @brief Like SENSORRING_REGISTER_FUNCTION_ASYNC with an explicit human-readable name.
 */
#define SENSORRING_REGISTER_FUNCTION_ASYNC_NAMED(Cap, Func, Name) \
  register_function_async<Cap>(Func, Name)

/**
 * @def SENSORRING_REGISTER_CAPABILITY(Cap)
 * @brief Register this device as implementing capability Cap via ICapability<Cap> (use in constructor).
 * The device must implement ICapability<Cap>. Uses stringized Cap as the capability name.
 */
#define SENSORRING_REGISTER_CAPABILITY(Cap) \
  register_capability<Cap>(#Cap)

/**
 * @def SENSORRING_REGISTER_CAPABILITY_NAMED(Cap, Name)
 * @brief Like SENSORRING_REGISTER_CAPABILITY with an explicit human-readable name.
 */
#define SENSORRING_REGISTER_CAPABILITY_NAMED(Cap, Name) \
  register_capability<Cap>(Name)

/**
 * @def SENSORRING_REGISTER_CAPABILITY_ASYNC(Cap)
 * @brief Register this device as implementing capability Cap via ICapabilityAsync<Cap> (use in constructor).
 */
#define SENSORRING_REGISTER_CAPABILITY_ASYNC(Cap) \
  register_capability_async<Cap>(#Cap)

/**
 * @def SENSORRING_REGISTER_CAPABILITY_ASYNC_NAMED(Cap, Name)
 * @brief Like SENSORRING_REGISTER_CAPABILITY_ASYNC with an explicit human-readable name.
 */
#define SENSORRING_REGISTER_CAPABILITY_ASYNC_NAMED(Cap, Name) \
  register_capability_async<Cap>(Name)

} // namespace device
} // namespace eduart
