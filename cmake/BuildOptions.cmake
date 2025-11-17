#########################################################
# options

if(UNIX)
  option( USE_SOCKETCAN "Add support for Linux SocketCAN" ON)
endif()
option( USE_USBTINGO "Add support for the USBtingo USB to CAN FD converter" OFF)
option( BUILD_SHARED_LIBS "Build as shared library" OFF)
option( BUILD_EXAMPLES "Build the example programs" OFF)
option( VERBOSE_BUILD "Verbose build" OFF)

if(WIN32)
  set( USE_USBTINGO ON)
endif()

if(NOT USE_SOCKETCAN AND NOT USE_USBTINGO)
  message(FATAL_ERROR "At least one of the options USE_SOCKETCAN or USE_USBTINGO has to be turned on!")
endif()