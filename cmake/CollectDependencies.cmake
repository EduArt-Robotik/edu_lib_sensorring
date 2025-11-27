if(USE_USBTINGO)
  find_package(usbtingo QUIET)

  if(NOT usbtingo_FOUND)
    message(STATUS "Did not find libusbtingo. Fetching it from GitHub...")

    include(FetchContent)
    FetchContent_Declare(
      usbtingo
      URL      https://github.com/hannesduske/libusbtingo/archive/refs/heads/master.zip
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
    )
    
    FetchContent_MakeAvailable(usbtingo)
    add_library(usbtingo::usbtingo ALIAS usbtingo)

  endif()
endif()

find_package(Threads REQUIRED)