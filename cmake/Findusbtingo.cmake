# Finds libusbtingo and provides:
#   usbtingo_FOUND
#   usbtingo::usbtingo
#   SENSORRING_USBTINGO_INSTALLED

include_guard(GLOBAL)

if(NOT TARGET usbtingo::usbtingo AND NOT TARGET usbtingo)
  set(USBTINGO_INSTALL_DEV_COMPONENTS ON)
  set(USBTINGO_BUILD_SHARED_LIBS ${SENSORRING_BUILD_SHARED_LIBS})
  set(USBTINGO_BUILD_EXAMPLES OFF)
  set(USBTINGO_BUILD_UTILS OFF)
  set(USBTINGO_BUILD_TESTS OFF)
  set(USBTINGO_INSTALL ON)
endif()

# 1. Prefer local checkout/submodule
set(_SENSORRING_USBTINGO_LOCAL_DIR
  "${PROJECT_SOURCE_DIR}/external/libusbtingo"
)
if(NOT TARGET usbtingo::usbtingo AND NOT TARGET usbtingo)
  if(EXISTS "${_SENSORRING_USBTINGO_LOCAL_DIR}/CMakeLists.txt")
    message(STATUS "libusbtingo: using local copy at ${_SENSORRING_USBTINGO_LOCAL_DIR}")
    add_subdirectory(
      "${_SENSORRING_USBTINGO_LOCAL_DIR}"
      "${CMAKE_CURRENT_BINARY_DIR}/usbtingo"
    )
  endif()
endif()

# 2. Try an installed package
if(NOT TARGET usbtingo::usbtingo AND NOT TARGET usbtingo)
  find_package(usbtingo CONFIG QUIET)
endif()

# 3. Fetch fallback
if(NOT TARGET usbtingo::usbtingo AND NOT TARGET usbtingo)
  message(STATUS "libusbtingo: not found locally, fetching from GitHub...")
  include(FetchContentCompat)
  fetchcontent_declare_compat(
    usbtingo
    URL https://github.com/hannesduske/libusbtingo/archive/v1.1.4.zip
  )
  FetchContent_MakeAvailable(usbtingo)
endif()

if(TARGET usbtingo AND NOT TARGET usbtingo::usbtingo)
  add_library(usbtingo::usbtingo ALIAS usbtingo)
endif()

set(_SENSORRING_USBTINGO_TARGET_FOUND FALSE)
set(SENSORRING_USBTINGO_INSTALLED OFF)
if(TARGET usbtingo::usbtingo)
  set(_SENSORRING_USBTINGO_TARGET_FOUND TRUE)
  get_target_property(SENSORRING_USBTINGO_INSTALLED usbtingo::usbtingo IMPORTED)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(usbtingo
  REQUIRED_VARS _SENSORRING_USBTINGO_TARGET_FOUND
)
mark_as_advanced(usbtingo_FOUND)
