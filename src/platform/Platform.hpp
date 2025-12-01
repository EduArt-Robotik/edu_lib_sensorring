#pragma once

#include <cstdlib>
#include <string>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

namespace eduart
{

static inline std::string getEnvVar(const char* name) {
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4996) // allow getenv on MSVC
#endif

    const char* val = std::getenv(name);

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

    return val ? std::string(val) : std::string();
}

}