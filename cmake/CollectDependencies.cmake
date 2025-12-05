if(SENSORRING_USE_USBTINGO)
  find_package(usbtingo QUIET)

  if(usbtingo_FOUND)
    set(SENSORRING_USBTINGO_INSTALLED ON)
  else()
    message(STATUS "Did not find libusbtingo. Fetching it from GitHub...")
    set(SENSORRING_USBTINGO_INSTALLED OFF)

    include(FetchContent)
    FetchContent_Declare(
      usbtingo
      URL      https://github.com/hannesduske/libusbtingo/archive/refs/heads/master.zip
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      EXCLUDE_FROM_ALL
    )

    set(USBTINGO_BUILD_SHARED_LIBS OFF)
    set(USBTINGO_BUILD_EXAMPLES OFF)
    set(USBTINGO_BUILD_UTILS OFF)
    set(USBTINGO_BUILD_TESTS OFF)
    set(USBTINGO_INSTALL OFF)

    FetchContent_MakeAvailable(usbtingo)

    # Do NOT install usbtingo
    set_property(TARGET usbtingo PROPERTY
      EXCLUDE_FROM_ALL ON
      EXCLUDE_FROM_DEFAULT_BUILD ON
    )

    add_library(usbtingo::usbtingo ALIAS usbtingo)   

  endif()
endif()

find_package(Threads REQUIRED)