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

    set(USBTINGO_INSTALL_DEV_COMPONENTS OFF)
    set(USBTINGO_BUILD_SHARED_LIBS ${SENSORRING_BUILD_SHARED_LIBS})
    set(USBTINGO_BUILD_EXAMPLES OFF)
    set(USBTINGO_BUILD_UTILS OFF)
    set(USBTINGO_BUILD_TESTS OFF)
    set(USBTINGO_INSTALL ON)
    
    FetchContent_MakeAvailable(usbtingo)
    add_library(usbtingo::usbtingo ALIAS usbtingo)   

  endif()
endif()

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