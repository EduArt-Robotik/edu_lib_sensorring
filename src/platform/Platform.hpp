#pragma once

#include <cstdlib>
#include <string>

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