#pragma once

#ifdef TRACY_ENABLE
// Profiler activated

#include "Tracy.hpp"
#define EduProfilingScope(X) ZoneScopedN(X);
#define EduFrameMark FrameMark;

#else
// Profiler deactivated

#define EduProfilingScope(X) ;
#define EduFrameMark ;

#endif