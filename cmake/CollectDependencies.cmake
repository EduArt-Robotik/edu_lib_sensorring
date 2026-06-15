#########################################################
# Transport module (submodule or FetchContent fallback)
if(EXISTS "${PROJECT_SOURCE_DIR}/external/edu_sensorring_transport/CMakeLists.txt")
  message(STATUS "Transport submodule found")
  set(SENSORRING_TRANSPORT_SOURCE_DIR
    "${PROJECT_SOURCE_DIR}/external/edu_sensorring_transport"
  )
  add_subdirectory(external/edu_sensorring_transport)
else()
  message(STATUS "Transport submodule not found, fetching from GitHub...")
  include(FetchContentCompat)
  fetchcontent_declare_compat(
    edu_sensorring_transport
    URL https://github.com/EduArt-Robotik/edu_sensorring_transport/archive/refs/heads/master.zip
  )
  FetchContent_MakeAvailable(edu_sensorring_transport)
  set(SENSORRING_TRANSPORT_SOURCE_DIR
    "${edu_sensorring_transport_SOURCE_DIR}"
  )
endif()

#########################################################
# Frankly bootloader (submodule or FetchContent fallback)
if(SENSORRING_BUILD_FIRMWARE_UPDATE)
  if(EXISTS "${PROJECT_SOURCE_DIR}/external/frankly_bootloader/CMakeLists.txt")
    message(STATUS "Bootloader submodule found")
    set(SENSORRING_BOOTLOADER_SOURCE_DIR
      "${PROJECT_SOURCE_DIR}/external/frankly_bootloader"
    )
  else()
    message(STATUS "Bootloader submodule not found, fetching from GitHub...")
    include(FetchContentCompat)
    fetchcontent_declare_compat(
      frankly_bootloader
      URL https://github.com/EduArt-Robotik/frankly_bootloader/archive/refs/heads/master.zip
    )
    FetchContent_GetProperties(frankly_bootloader)
    if(NOT frankly_bootloader_POPULATED)
      FetchContent_Populate(frankly_bootloader)
    endif()
    set(SENSORRING_BOOTLOADER_SOURCE_DIR
      "${frankly_bootloader_SOURCE_DIR}"
    )
  endif()
endif()

#########################################################
# USBtingo (optional dependency)
if(SENSORRING_USE_USBTINGO)
  find_package(usbtingo QUIET)

  if(usbtingo_FOUND)
    message(STATUS "Dependency libusbtingo is installed")
    set(SENSORRING_USBTINGO_INSTALLED ON)
  else()
    message(STATUS "Dependency libusbtingo was not found, fetching it from GitHub...")
    set(SENSORRING_USBTINGO_INSTALLED OFF)

    include(FetchContentCompat)
    fetchcontent_declare_compat(
      usbtingo
      URL https://github.com/hannesduske/libusbtingo/archive/v1.1.4.zip
      #URL https://github.com/hannesduske/libusbtingo/archive/refs/heads/develop.zip
    )

    set(USBTINGO_INSTALL_DEV_COMPONENTS ON)
    set(USBTINGO_BUILD_SHARED_LIBS ${SENSORRING_BUILD_SHARED_LIBS})
    set(USBTINGO_BUILD_EXAMPLES OFF)
    set(USBTINGO_BUILD_UTILS OFF)
    set(USBTINGO_BUILD_TESTS OFF)
    set(USBTINGO_INSTALL ON)
    
    FetchContent_MakeAvailable(usbtingo)
    add_library(usbtingo::usbtingo ALIAS usbtingo)   

  endif()
endif()

#########################################################
# Catch2 (optional dependency for tests)
if(SENSORRING_BUILD_TESTS)
  find_package(Catch2 QUIET)

  if(Catch2_FOUND)
    message(STATUS "Dependency Catch2 is installed")
    set(SENSORRING_CATCH2_INSTALLED ON)
  else()
    message(STATUS "Dependency Catch2 was not found, fetching it from GitHub...")
    set(SENSORRING_CATCH2_INSTALLED OFF)

    include(FetchContentCompat)
    fetchcontent_declare_compat(
      Catch2
      URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.zip
    )

    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

  endif()
endif()

find_package(Threads REQUIRED)