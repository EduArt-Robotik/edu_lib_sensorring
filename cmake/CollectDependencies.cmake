if(SENSORRING_USE_USBTINGO)
  find_package(usbtingo QUIET)

  if(usbtingo_FOUND)
    set(SENSORRING_USBTINGO_INSTALLED ON)
  else()
    message(STATUS "Did not find libusbtingo. Fetching it from GitHub...")
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
  include(FetchContentCompat)
  fetchcontent_declare_compat(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.zip
  )
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

find_package(Threads REQUIRED)