# Finds frankly_bootloader and provides:
#   frankly_bootloader_FOUND
#   frankly_bootloader::frankly_bootloader (for an installed package)
#   SENSORRING_BOOTLOADER_SOURCE_DIR (when using source files)

include_guard(GLOBAL)

# 1. Prefer local checkout/submodule
set(_SENSORRING_BOOTLOADER_LOCAL_DIR
  "${PROJECT_SOURCE_DIR}/external/frankly_bootloader"
)
if(EXISTS "${_SENSORRING_BOOTLOADER_LOCAL_DIR}/CMakeLists.txt")
  message(STATUS "frankly_bootloader: using local copy at ${_SENSORRING_BOOTLOADER_LOCAL_DIR}")
  set(SENSORRING_BOOTLOADER_SOURCE_DIR "${_SENSORRING_BOOTLOADER_LOCAL_DIR}")
endif()

# 2. Try an installed package
if(NOT SENSORRING_BOOTLOADER_SOURCE_DIR)
  find_package(frankly_bootloader CONFIG QUIET
    NAMES frankly_bootloader frankly-bootloader
  )
  if(TARGET frankly-bootloader AND NOT TARGET frankly_bootloader::frankly_bootloader)
    add_library(frankly_bootloader::frankly_bootloader ALIAS frankly-bootloader)
  endif()
endif()

# 3. Fetch fallback
if(NOT SENSORRING_BOOTLOADER_SOURCE_DIR AND NOT TARGET frankly_bootloader::frankly_bootloader)
  message(STATUS "frankly_bootloader: not found locally, fetching from GitHub...")
  include(FetchContentCompat)
  fetchcontent_declare_compat(
    frankly_bootloader
    URL https://github.com/EduArt-Robotik/frankly_bootloader/archive/refs/heads/master.zip
  )
  # Only populate sources: the upstream project also builds tests and device utilities.
  FetchContent_GetProperties(frankly_bootloader)
  if(NOT frankly_bootloader_POPULATED)
    FetchContent_Populate(frankly_bootloader)
  endif()
  set(SENSORRING_BOOTLOADER_SOURCE_DIR "${frankly_bootloader_SOURCE_DIR}")
endif()

set(_SENSORRING_BOOTLOADER_FOUND FALSE)
if(SENSORRING_BOOTLOADER_SOURCE_DIR)
  if(EXISTS "${SENSORRING_BOOTLOADER_SOURCE_DIR}/src/francor/franklyboot/msg.cpp"
      AND IS_DIRECTORY "${SENSORRING_BOOTLOADER_SOURCE_DIR}/include")
    set(_SENSORRING_BOOTLOADER_FOUND TRUE)
  endif()
elseif(TARGET frankly_bootloader::frankly_bootloader)
  set(_SENSORRING_BOOTLOADER_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(frankly_bootloader
  REQUIRED_VARS _SENSORRING_BOOTLOADER_FOUND
)
mark_as_advanced(frankly_bootloader_FOUND)
