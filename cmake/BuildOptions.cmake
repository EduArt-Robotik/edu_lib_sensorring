#########################################################
# options

if(IS_LINUX)
  option( SENSORRING_USE_SOCKETCAN "Compile with support for Linux SocketCAN" ON)
endif()
option( SENSORRING_USE_USBTINGO "Compile with support for the USBtingo USB adapter" OFF)
option( SENSORRING_BUILD_SHARED_LIBS "Build as shared library" OFF)
option( SENSORRING_BUILD_EXAMPLES "Build the example programs" OFF)
option( SENSORRING_BUILD_DOCUMENTATION "Build the documentation" OFF)
option( SENSORRING_BUILD_PYTHON_BINDINGS "Build python bindings" OFF)

if(IS_WINDOWS)
  set( SENSORRING_USE_USBTINGO ON)
endif()

if(NOT SENSORRING_USE_SOCKETCAN AND NOT SENSORRING_USE_USBTINGO)
  message(FATAL_ERROR "At least one of the options USE_SOCKETCAN or USE_USBTINGO has to be turned on!")
endif()