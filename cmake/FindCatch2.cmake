# Finds Catch2 and provides:
#   Catch2_FOUND
#   Catch2::Catch2WithMain
#   SENSORRING_CATCH2_INSTALLED
#   Catch.cmake test discovery helpers via CMAKE_MODULE_PATH

include_guard(GLOBAL)

# 1. Prefer local checkout/submodule
set(_SENSORRING_CATCH2_LOCAL_DIR
  "${PROJECT_SOURCE_DIR}/external/Catch2"
)
if(NOT TARGET Catch2::Catch2WithMain)
  if(EXISTS "${_SENSORRING_CATCH2_LOCAL_DIR}/CMakeLists.txt")
    message(STATUS "Catch2: using local copy at ${_SENSORRING_CATCH2_LOCAL_DIR}")
    add_subdirectory(
      "${_SENSORRING_CATCH2_LOCAL_DIR}"
      "${CMAKE_CURRENT_BINARY_DIR}/Catch2"
    )
    list(APPEND CMAKE_MODULE_PATH "${_SENSORRING_CATCH2_LOCAL_DIR}/extras")
  endif()
endif()

# 2. Try an installed package
if(NOT TARGET Catch2::Catch2WithMain)
  find_package(Catch2 3 CONFIG QUIET)
endif()

# 3. Fetch fallback
if(NOT TARGET Catch2::Catch2WithMain)
  message(STATUS "Catch2: not found locally, fetching from GitHub...")
  include(FetchContentCompat)
  fetchcontent_declare_compat(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.zip
  )
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()

set(_SENSORRING_CATCH2_TARGET_FOUND FALSE)
set(SENSORRING_CATCH2_INSTALLED OFF)
if(TARGET Catch2::Catch2WithMain)
  set(_SENSORRING_CATCH2_TARGET_FOUND TRUE)
  get_target_property(SENSORRING_CATCH2_INSTALLED Catch2::Catch2WithMain IMPORTED)
  if(SENSORRING_CATCH2_INSTALLED)
    if(Catch2_DIR)
      list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
    endif()
  else()
    # Also support a Catch2 target already added by the parent project.
    get_target_property(_SENSORRING_CATCH2_SOURCE_DIR Catch2::Catch2WithMain SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH "${_SENSORRING_CATCH2_SOURCE_DIR}/../extras")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Catch2
  REQUIRED_VARS _SENSORRING_CATCH2_TARGET_FOUND
)
mark_as_advanced(Catch2_FOUND)
