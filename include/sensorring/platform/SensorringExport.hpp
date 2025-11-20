// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   Api.hpp
 * @author EduArt Robotik GmbH
 * @brief  Control the import and export of Windows DLL symbols
 * @date   2025-11-14
 */

#pragma once

// Windows building a shared lib (.dll)
#if defined(SENSORRING_SHARED) && (defined(_WIN32) || defined(_WIN64))
// Disable compiler warning C4251 caused by members of standard library
// https://learn.microsoft.com/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251
#pragma warning(disable : 4251)

#ifdef SENSORRING_DLL_EXPORT
#define SENSORRING_API __declspec(dllexport)
#else
#define SENSORRING_API __declspec(dllimport)
#endif

// Linux building a shared lib (.so)
// Linux building a static lib (.a)
// Windows building a static lib (.lib)
#else
#define SENSORRING_API
#endif