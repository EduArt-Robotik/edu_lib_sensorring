#########################################################
# options

if(IS_LINUX)
  option( USE_SOCKETCAN "Compile with support for Linux SocketCAN" ON)
endif()
option( USE_USBTINGO "Compile with support for the USBtingo USB adapter" OFF)
option( BUILD_SHARED_LIBS "Build as shared library" OFF)
option( BUILD_EXAMPLES "Build the example programs" OFF)
option( BUILD_DOCUMENTATION "Build the documentation" OFF)
option( BUILD_PYTHON_BINDINGS "Build python bindings" OFF)

if(IS_WINDOWS)
  set( USE_USBTINGO ON)
endif()

if(NOT USE_SOCKETCAN AND NOT USE_USBTINGO)
  message(FATAL_ERROR "At least one of the options USE_SOCKETCAN or USE_USBTINGO has to be turned on!")
endif()