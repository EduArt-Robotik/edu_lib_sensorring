#########################################################
# Configure install paths

set(IS_WINDOWS FALSE)
set(IS_LINUX FALSE)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(IS_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(IS_LINUX TRUE)
else()
  message(FATAL_ERROR "Unknown target system. Aborting cmake configuration.")
endif()