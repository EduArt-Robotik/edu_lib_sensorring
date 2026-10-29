# Finds edu_sensorring_transport and provides:
#   sensorring_transport_FOUND
#   sensorring_transport::sensorring_transport
#   SENSORRING_TRANSPORT_SOURCE_DIR (when built from source)

include_guard(GLOBAL)

# 1. Prefer local checkout/submodule
set(_SENSORRING_TRANSPORT_LOCAL_DIR
  "${PROJECT_SOURCE_DIR}/external/edu_sensorring_transport"
)
if(NOT TARGET sensorring_transport::sensorring_transport AND NOT TARGET sensorring_transport)
  if(EXISTS "${_SENSORRING_TRANSPORT_LOCAL_DIR}/CMakeLists.txt")
    message(STATUS "edu_sensorring_transport: using local copy at ${_SENSORRING_TRANSPORT_LOCAL_DIR}")
    set(SENSORRING_TRANSPORT_SOURCE_DIR "${_SENSORRING_TRANSPORT_LOCAL_DIR}")
    add_subdirectory(
      "${SENSORRING_TRANSPORT_SOURCE_DIR}"
      "${CMAKE_CURRENT_BINARY_DIR}/sensorring_transport"
    )
  endif()
endif()

# 2. Try an installed package
if(NOT TARGET sensorring_transport::sensorring_transport AND NOT TARGET sensorring_transport)
  find_package(sensorring_transport CONFIG QUIET)
endif()

# 3. Fetch fallback
if(NOT TARGET sensorring_transport::sensorring_transport AND NOT TARGET sensorring_transport)
  message(STATUS "edu_sensorring_transport: not found locally, fetching from GitHub...")
  include(FetchContentCompat)
  fetchcontent_declare_compat(
    edu_sensorring_transport
    URL https://github.com/EduArt-Robotik/edu_sensorring_transport/archive/refs/heads/master.zip
  )
  FetchContent_MakeAvailable(edu_sensorring_transport)
  set(SENSORRING_TRANSPORT_SOURCE_DIR "${edu_sensorring_transport_SOURCE_DIR}")
endif()

if(TARGET sensorring_transport AND NOT TARGET sensorring_transport::sensorring_transport)
  add_library(sensorring_transport::sensorring_transport ALIAS sensorring_transport)
endif()

set(_SENSORRING_TRANSPORT_TARGET_FOUND FALSE)
if(TARGET sensorring_transport::sensorring_transport)
  set(_SENSORRING_TRANSPORT_TARGET_FOUND TRUE)
  get_target_property(_SENSORRING_TRANSPORT_IMPORTED
    sensorring_transport::sensorring_transport IMPORTED
  )
  if(NOT _SENSORRING_TRANSPORT_IMPORTED)
    get_target_property(SENSORRING_TRANSPORT_SOURCE_DIR
      sensorring_transport::sensorring_transport SOURCE_DIR
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sensorring_transport
  REQUIRED_VARS _SENSORRING_TRANSPORT_TARGET_FOUND
)
mark_as_advanced(sensorring_transport_FOUND)
